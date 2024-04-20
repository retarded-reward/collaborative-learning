//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "controller.h"
#include "agentc/agent_client.h"
#include "ActionRequest_m.h"
#include "SimulationMsg_m.h"
#include "NodeStateMsg_m.h"
#include "power/battery.h"

Define_Module(Controller);

/*********************** TEST METHODS - BEGIN **************************************/

void Controller::send_test_action_request(){
    // Tests the agent client by sending it a request for an action.
    // Normal workflow should expect a response from the agent client containing
    // the action to take.

    NodeStateMsg *neighbour = new NodeStateMsg();
    neighbour->setEnergy(100);
    neighbour->setHas_packet_in_buffer(false);
    neighbour->setPower_state(NodePowerState::ON);

    ActionRequest *ar = new ActionRequest();

    ar->getStateForUpdate().setEnergy(100);
    ar->getStateForUpdate().setHas_packet_in_buffer(false);
    ar->getStateForUpdate().setPower_state(NodePowerState::OFF);
    ar->getStateForUpdate().appendNeighbour(neighbour);

    ar->setRewardArraySize(1);
    ar->getRewardForUpdate(0).setMessage_id(1);
    ar->getRewardForUpdate(0).setValue(10);

    send(ar, "agent_port$o");
}

/*********************** TEST METHODS - END **************************************/

//Node behaviour when started
void Controller::initialize()
{
    init_module_params();
    init_ask_action_timer();
    init_power_source();
    init_data_buffer();
    init_power_state();
    
    /** TESTS ******/
    //send_test_action_request();
    /** TESTS (END)*/

    start_timer(ask_action_timeout);

}

void Controller::ask_action(){

    ActionRequest *ar;
    
    EV_DEBUG << "Asking for action" << endl;

    // sample state in a action request object and send it to the agent client
    ar = new ActionRequest();
    sample_state(ar->getStateForUpdate());

    // TODO: compute reward and write it in the action request 

    send(ar, "agent_port$o");

}

void Controller::do_action(ActionResponse *action)
{
    // TODO: implement this method
    EV_DEBUG << "Doing action" << endl;
}

void Controller::start_timer(Timeout *timeout)
{
    EV_DEBUG << "Starting timer at " << simTime() << endl;
    EV_DEBUG << "Timeout delta: " << timeout->getDelta() << endl;
    scheduleAfter(timeout->getDelta(), timeout);
}

void Controller::stop_timer(Timeout *timeout)
{
    EV_DEBUG << "Stopping timer at " << simTime() << endl;
    cancelEvent(timeout);
}

void Controller::sample_state(NodeStateMsg &state)
{    
    NodeStateMsg *neighbour_state_msg;
    NeighbourState neighbour_state;
    
    state.setEnergy(power_source->getCharge());
    state.setHas_packet_in_buffer(data_buffer->getLength() > 0);
    state.setId(id);
    state.setPower_state(power_state);
    state.setNeighbourArraySize(neighbours.size());
    state.setData_cap(link_cap);
    for (int i = 0; i < neighbours.size(); i++){
        neighbour_state = neighbours.at(i);
        neighbour_state_msg = new NodeStateMsg();
        *neighbour_state_msg = *neighbour_state.state;
        state.appendNeighbour(neighbour_state_msg);
    }
}

void Controller::init_ask_action_timer()
{
    this->ask_action_timeout = new Timeout(
        TimeoutKind::ASK_ACTION, ask_action_timeout_delta);
}

void Controller::init_module_params()
{
    ask_action_timeout_delta = par("ask_action_timeout_delta").intValue();
    battery_capacity = par("battery_capacity").doubleValue();
    data_buffer_capacity = par("data_buffer_capacity").intValue();
    max_neighbours = par("max_neighbours").intValue();
    id = par("id").intValue();
    link_cap = par("link_cap").doubleValue();
    // add more module params here ...
}

void Controller::init_power_source()
{
    power_source =  new Battery(battery_capacity);
    power_source->plug();
}

void Controller::init_data_buffer()
{
    data_buffer = new FixedCapCQueue(data_buffer_capacity);
}

void Controller::init_neighbours()
{
    //neighbours = new vector<NeighbourState>();
    neighbours.reserve(max_neighbours);
}

void Controller::init_power_state()
{
    power_state = NodePowerState::OFF;
}

void Controller::handleActionResponse(ActionResponse *msg)
{    
    EV_DEBUG << "Action response received";
    EV_DEBUG  << "Change power state: " << msg->getChange_power_state() << endl;

    do_action(msg);
    start_timer(ask_action_timeout);

}

void Controller::handleDataMsg(DataMsg *msg)
{
    // TODO: implement data buffering
    
    EV << "Data received "<< msg->getData();

    // TODO: implement data forwarding
    
    // Asks action after receiving data and resets action timer
    stop_timer(ask_action_timeout);
    ask_action();

}

void Controller::handleAskActionTimeout(Timeout *msg)
{
    EV_DEBUG << "Ask action timeout expired at " << simTime() << endl;
    ask_action();
}

void Controller::handleRewardMsg(RewardMsg *msg)
{
    //TODO implement this method
    EV << "Received reward: " << msg->getSink_reward().getValue() << " for message id: " << msg->getSink_reward().getMessage_id() << endl;
}

//Node behaviour at message reception
void Controller::handleMessage(cMessage *msg)
{
    EV << "Message received";

    if (is_agentc_msg(msg)){
        switch (msg->getKind())
        {
        case (int) AgentClientMsgKind::ACTION_RESPONSE:
            handleActionResponse((ActionResponse *) msg);
            break;
        
        default:
            EV_ERROR << "Controller: unrecognized agentc message kind " 
            << msg->getKind() << endl;
            break;
        }
    }
    else if(is_sim_msg(msg)){
        switch (msg->getKind())
        {
        case (int) SimulationMsgKind::DATA_MSG:
            handleDataMsg((DataMsg *) msg);
            break;
        case (int) SimulationMsgKind::REWARD_MSG:
            handleRewardMsg((RewardMsg *) msg);
            break;
        //Add cases for other message types
        default:
            EV_ERROR << "Controller: unrecognized simulation message kind " 
            << msg->getKind() << endl;
            break;
        }
    }
    else if (is_timeout_msg(msg)){
        switch (msg->getKind())
        {
        case (int) TimeoutKind::ASK_ACTION:
            handleAskActionTimeout((Timeout *)msg);
            break;
        default:
            EV_ERROR << "Controller: unrecognized timeoutkind " 
            << msg->getKind() << endl;
            break;
        }
        // do not delete timeout messages
        goto handleMessage_do_not_delete_msg;
    }
    else{
        EV_ERROR << "Controller: message name is not an expected topic: "
         << msg->getName() << endl;
    }
        
    delete msg;

handleMessage_do_not_delete_msg:
    return;

}

Controller::~Controller()
{
    cancelAndDelete(ask_action_timeout);
    delete power_source;
    delete data_buffer;
}
