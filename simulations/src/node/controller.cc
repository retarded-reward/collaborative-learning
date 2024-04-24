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
#include "SinkRewardMsg_m.h"
#include "power/battery.h"
#include "power/power_chord.h"
#include <vector>

Define_Module(Controller);

//Node behaviour when started
void Controller::initialize()
{
    init_reward_params();
    init_module_params();
    init_ask_action_timer();
    init_power_sources();
    init_data_buffer();
    start_timer(ask_action_timeout);

}

void Controller::ask_action(){

    ActionRequest *ar;
    
    EV_DEBUG << "Asking for action" << endl;

    // sample state in a action request object and send it to the agent client
    ar = new ActionRequest();
    sample_state(ar->getStateForUpdate());

    float reward=compute_reward();

    //TODO Embed reward in the action request

    send(ar, "agent_port$o");

}

float Controller::compute_reward(){
    // Compute reward and write it in the action request
    float queue_term=queue_occ_cost*queue_occ;

    float energy_term=energy_cost*energy_consumed;
    energy_consumed=0;

    float pkt_drop_term=pkt_drop_cost*pkt_drop_cnt;
    pkt_drop_cnt=0;

    float reward=pkt_drop_penalty_weight*pkt_drop_term+queue_occ_penalty_weight*queue_term+energy_penalty_weight*energy_term;
    return reward;
}

void Controller::do_action(ActionResponse *action)
{
    EV_DEBUG << "Doing action" << endl;

    // TODO: implement this method
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
    // TODO: implement this method    
}

void Controller::init_ask_action_timer()
{
    this->ask_action_timeout = new Timeout(
        TimeoutKind::ASK_ACTION, ask_action_timeout_delta);
}

void Controller::init_reward_params()
{
    pkt_drop_penalty_weight = par("pkt_drop_penalty_weight").doubleValue();
    queue_occ_penalty_weight = par("queue_occ_penalty_weight").doubleValue();
    energy_penalty_weight = par("energy_penalty_weight").doubleValue();
    pkt_drop_cost = par("pkt_drop_cost").doubleValue();
    queue_occ_cost = par("queue_occ_cost").doubleValue();
    energy_cost = par("energy_cost").doubleValue();
    pkt_drop_cnt=0;
    queue_occ=0;
    energy_consumed=0;
}


void Controller::init_module_params()
{   
    ask_action_timeout_delta = par("ask_action_timeout_delta").intValue();
    data_buffer_capacity = par("data_buffer_capacity").intValue();
    max_neighbours = par("max_neighbours").intValue();
    link_cap = par("link_cap").doubleValue();
    power_model = new NICPowerModel();
    power_models = (cValueMap *) par("power_models").objectValue()->dup();
    power_source_models = (cValueMap *) par("power_source_models").objectValue()->dup();
    
    EV_DEBUG << "Power model tx_mW: " << power_model->getTx_mW() << endl;
    // add more module params here ...
}

void Controller::init_data_buffer()
{
        data_buffer=new FixedCapCQueue(data_buffer_capacity);
}

void Controller::init_neighbours()
{
    neighbours.reserve(max_neighbours);
}

void Controller::init_power_sources()
{
    cValueMap *battery_params = (cValueMap *) power_source_models->get("belkin_BPB001_powerbank").objectValue();
    cValueMap *power_chord_params = (cValueMap *) power_source_models->get("power_chord_standard").objectValue();
    
    battery = new Battery(battery_params->get("cap_mWh").doubleValueInUnit("mWh"));
    battery->setCostPerMWh(battery_params->get("cost_per_mWh").doubleValue());
    EV_DEBUG << "Battery capacity: " << battery->getCharge() << endl;
    EV_DEBUG << "Battery cost per mWh: " << battery->getCostPerMWh() << endl;

    power_chord = new PowerChord();
    power_chord->setCostPerMWh(power_chord_params->get("cost_per_mWh").doubleValue());
    EV_DEBUG << "Power chord cost per mWh: " << power_chord->getCostPerMWh() << endl;

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
    
    EV << "Data received inserting into buffer msg_id="<< msg->getMsg_id();
    try{
        //TODO With priority queue choose queue
        data_buffer->insert(msg);
    }
    catch (std::out_of_range &e){
        EV_ERROR << "Data buffer is full, dropping message" << endl;
        //TODO With priority queue choose queue
        pkt_drop_cnt++;
        EV_ERROR<< "Packet drop count=" << pkt_drop_cnt <<endl;
    }
    
}

void Controller::handleAskActionTimeout(Timeout *msg)
{
    EV_DEBUG << "Ask action timeout expired at " << simTime() << endl;
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
    
    delete battery;
    delete power_chord;
    delete power_model;
    delete data_buffer;
    delete power_models;
    delete power_source_models;
}
