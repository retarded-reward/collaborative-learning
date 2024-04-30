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
#include "power/power_chord.h"
#include "power/random_charger.h"
#include "QueueDataRequest_m.h"
#include <cstddef>

Define_Module(Controller);

//Node behaviour when started
void Controller::initialize()
{
    init_reward_params();
    init_module_params();
    init_timers();
    init_power_sources();
    init_queue_states();
    
    start_timer(ask_action_timeout);
    start_timer(charge_battery_timeout);
}

void Controller::ask_action(){

    ActionRequest *ar;
    float reward;
    
    EV_DEBUG << "Asking for action" << endl;

    // sample state in a action request object and send it to the agent client
    ar = new ActionRequest();
    sample_state(ar->getStateForUpdate());

    // compute reward and write it in the action request
    sample_reward(ar->getRewardForUpdate());

    EV_DEBUG<< "sending state:" << endl;
    EV_DEBUG<< "battery level: " << ar->getState().getEnergy_percentage() << endl;
    EV_DEBUG << "charge rate: " << ar->getState().getCharge_rate_percentage() << endl;
    for (int i = 0; i < ar->getState().getQueue_pop_percentageArraySize(); i ++){
        EV_DEBUG << "queue  "<< i<< "full at " 
         << ar->getState().getQueue_pop_percentage(i) << "%" << endl;
    }
    send(ar, "agent_port$o");

}

reward_t Controller::compute_reward(){
    // Compute reward and write it in the action request
    reward_t queue_term=queue_occ_cost*queue_occ;

    reward_t energy_term=energy_cost*energy_consumed;
    energy_consumed=0;

    reward_t pkt_drop_term=pkt_drop_cost*pkt_drop_cnt;
    pkt_drop_cnt=0;

    reward_t reward=pkt_drop_penalty_weight*pkt_drop_term+queue_occ_penalty_weight*queue_term+energy_penalty_weight*energy_term;
    return reward;
}

void Controller::update_queue_state(QueueStateUpdate *msg)
{
    size_t queue_idx; 
    
    queue_idx = msg->getArrivalGate()->getIndex();
    queue_states[queue_idx].occupancy = msg->getBuffer_pop_percentage();
}

void Controller::charge_battery()
{
    mWh_t charge;
    PowerSource *battery;

    // tries to charge battery by the maximum amount the charger is able to output.
    // The actual amount depends on the inner distribution of the RandomCharger.
    charge = battery_charger->discharge(battery_charger->getCapacity());
    battery = power_sources[SelectPowerSource::BATTERY];
    battery->recharge(charge);

    // updates last charge rate percentage
    last_charge_rate = calc_percentage(charge, battery->getCapacity()); 

    EV_DEBUG << "battery charger outputted " << charge << "mWh" << endl;
}

void Controller::do_action(ActionResponse *action)
{
    // TODO: implement this method
    EV_DEBUG << "Action: send_message: " << action->getSend_message() << endl;
    EV_DEBUG << "Action: select_power_source: " << action->getSelect_power_source() << endl;
    EV_DEBUG << "Action: queue: " << action->getQueue() << endl;

    /*
    Test code to request data from queue.

    QueueDataRequest *queueDataRequest = new QueueDataRequest();
    queueDataRequest->setData_n(100);
    send(queueDataRequest, "queue_ports$o", action->getQueue());
    */
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
    sample_power_sources(state);
    sample_queue_states(state);
}

void Controller::sample_power_sources(NodeStateMsg &state_msg)
{
    percentage_t battery_level;
    PowerSource *battery;

    // sample battery level
    battery = power_sources[SelectPowerSource::BATTERY];
    battery_level = calc_percentage(battery->getCharge(), battery->getCapacity());
    state_msg.setEnergy_percentage(battery_level);

    // samples last measured battery charge rate
    state_msg.setCharge_rate_percentage(last_charge_rate);
}

void Controller::sample_queue_states(NodeStateMsg &state_msg)
{
    for (struct QueueState &qstate : queue_states){
        state_msg.appendQueue_pop_percentage(qstate.occupancy);
    }
}

void Controller::sample_reward(RewardMsg &reward_msg)
{
    reward_msg.setValue(compute_reward());
}

void Controller::init_timers()
{
    charge_battery_timeout = new Timeout();
    charge_battery_timeout->setKind(TimeoutKind::CHARGE_BATTERY);
    charge_battery_timeout->setDelta(charge_battery_timeout_delta);

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
    ask_action_timeout_delta = par("ask_action_timeout_delta").doubleValue();
    data_buffer_capacity = par("data_buffer_capacity").intValue();
    max_neighbours = par("max_neighbours").intValue();
    link_cap = par("link_cap").doubleValue();
    power_model = new NICPowerModel();
    power_models = (cValueMap *) par("power_models").objectValue()->dup();
    power_source_models = (cValueMap *) par("power_source_models").objectValue()->dup();
    num_queues = getParentModule()->par("num_queues").intValue();
    charge_battery_timeout_delta 
     = par("charge_battery_timeout_delta").doubleValue();
    EV_DEBUG << "charge battery timeout delta: "<< charge_battery_timeout_delta << endl;
    EV_DEBUG << "ask action timeout delta: " << ask_action_timeout_delta << endl;
    EV_DEBUG << "Power model tx_mW: " << power_model->getTx_mW() << endl;
    // add more module params here ...
}

void Controller::init_power_sources()
{
    const size_t num_power_sources = 2;
    
    cValueMap *battery_params
     = (cValueMap *) power_source_models->get("belkin_BPB001_powerbank").objectValue();
    cValueMap *power_chord_params
     = (cValueMap *) power_source_models->get("power_chord_standard").objectValue();
    cValueMap *battery_charger_params
     = (cValueMap *) power_source_models->get("solar_panel").objectValue();
    
    power_sources.resize(num_power_sources, (PowerSource *) nullptr);
    
    power_sources[SelectPowerSource::BATTERY]
     = new Battery(battery_params->get("cap_mWh").doubleValueInUnit("mWh"));
    power_sources[SelectPowerSource::BATTERY]
     ->setCostPerMWh(battery_params->get("cost_per_mWh").doubleValue());
    power_sources[SelectPowerSource::BATTERY]->plug();
    EV_DEBUG << "Battery capacity: " << power_sources[SelectPowerSource::BATTERY]->getCharge() << endl;
    EV_DEBUG << "Battery cost per mWh: " << power_sources[SelectPowerSource::BATTERY]->getCostPerMWh() << endl;

    power_sources[SelectPowerSource::POWER_CHORD] = new PowerChord();
    power_sources[SelectPowerSource::POWER_CHORD]
     ->setCostPerMWh(power_chord_params->get("cost_per_mWh").doubleValue());
    power_sources[SelectPowerSource::POWER_CHORD]->plug();
    EV_DEBUG << "Power chord cost per mWh: " << power_sources[SelectPowerSource::POWER_CHORD]->getCostPerMWh() << endl;

    // init battery charger
    cPar &charge_rate_distribution = par("battery_charge_rate_distribution");
    battery_charger = new RandomCharger(par("battery_charge_rate_distribution"),
     battery_charger_params->get("cap_mWh").doubleValueInUnit("mWh"));
    EV_DEBUG << "max charge is " << battery_charger->getCapacity() << endl;
    battery_charger->plug();
}

void Controller::init_queue_states()
{
    queue_states.resize(num_queues, (struct QueueState){0});  
}

void Controller::handleActionResponse(ActionResponse *msg)
{    
    EV_DEBUG << "Action response received";

    do_action(msg);
    start_timer(ask_action_timeout);

}

void Controller::handleDataMsg(DataMsg *msg)
{
    // TODO: implement this method
    EV_DEBUG << "Data message received" << endl;    
}

void Controller::handleAskActionTimeout(Timeout *msg)
{
    EV_DEBUG << "Ask action timeout expired at " << simTime() << endl;
    ask_action();
}

void Controller::handleChargeBatteryTimeout(Timeout *msg)
{
    charge_battery();
    EV_DEBUG << "battery charged at " << power_sources[SelectPowerSource::BATTERY]->getCharge() << endl;
        
    start_timer(charge_battery_timeout);
}

void Controller::handleQueueDataResponse(QueueDataResponse *msg)
{
    // TODO: implement this method

    EV_DEBUG << "Queue data response size: " << msg->getDataArraySize() << endl;
}

void Controller::handleQueueStateUpdate(QueueStateUpdate *msg)
{    
    update_queue_state(msg);

    // TODO: track num of dropped packets in order to use it to compute reward

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
        case (int) TimeoutKind::CHARGE_BATTERY:
            handleChargeBatteryTimeout((Timeout *)msg);
            break;
        default:
            EV_ERROR << "Controller: unrecognized timeoutkind " 
            << msg->getKind() << endl;
            break;
        }
        // do not delete timeout messages
        goto handleMessage_do_not_delete_msg;
    }
    else if (is_queue_msg(msg)){
        switch (msg->getKind())
        {
        case (int) QueueMsgKind::QUEUE_DATA_RESPONSE:
            handleQueueDataResponse((QueueDataResponse *)msg);
            break;
        case (int) QueueMsgKind::QUEUE_STATE_UPDATE:
            handleQueueStateUpdate((QueueStateUpdate *)msg);
            break;
        default:
            break;
        }
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
    cancelAndDelete(charge_battery_timeout);
    
    delete power_model;
    delete power_models;
    delete power_source_models;
}
