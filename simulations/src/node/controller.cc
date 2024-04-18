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
    ar->getStateForUpdate().appendNeighbour(*neighbour);

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
    
    /** TESTS ******/
    //send_test_action_request();
    /** TESTS (END)*/

    start_timer(ask_action_timeout);

}

void Controller::ask_action(){

    // TODO: implement this method
    EV_DEBUG << "Asking for action" << endl;

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

void Controller::init_ask_action_timer()
{
    this->ask_action_timeout = new Timeout(
        TimeoutKind::ASK_ACTION, ask_action_timeout_delta);
}

void Controller::init_module_params()
{
    this->ask_action_timeout_delta = par("ask_action_timeout_delta").intValue();
    // add more module params here ...
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

    // Asks action after receiving data and resets action timer
    stop_timer(ask_action_timeout);
    ask_action();

}

void Controller::handleAskActionTimeout(Timeout *msg)
{
    EV_DEBUG << "Ask action timeout received" << endl;
    ask_action();
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
}
