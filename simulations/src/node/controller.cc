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
#include "power/battery.h"
#include "power/power_chord.h"
#include "power/random_charger.h"
#include "QueueDataRequest_m.h"
#include <cstddef>

Define_Module(Controller);

//Node behaviour when started
void Controller::initialize()
{
    init_module_params();
    init_timers();
    init_power_sources();
    init_queue_states();
    init_reward_params();
    
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
    sample_reward(ar->getRewardForUpdate());//Todo mettere la last_reward

    EV_DEBUG<< "Sending state:" << endl;
    EV_DEBUG<< "Battery level: " << ar->getState().getEnergy_percentage() << endl;
    EV_DEBUG << "Charge rate: " << ar->getState().getCharge_rate_percentage() << endl;
    for (int i = 0; i < ar->getState().getQueue_pop_percentageArraySize(); i ++){
        EV_DEBUG << "Queue  "<< i<< " full at " 
         << ar->getState().getQueue_pop_percentage(i) << "%" << endl;
        
    }
    send(ar, "agent_port$o");

}

void Controller::update_queue_state(QueueStateUpdate *msg, size_t queue_idx)
{
    queue_states[queue_idx].occupancy = msg->getBuffer_pop_percentage();
    queue_states[queue_idx].pkt_drop_cnt += msg->getNum_of_dropped();
    EV_DEBUG << "Queue " << queue_idx << " state updated with occupancy: " 
    << queue_states[queue_idx].occupancy << "%" << " and pkt dropped: " 
    << queue_states[queue_idx].pkt_drop_cnt << endl;
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

/*
    Perform action, if action is "do nothing" do nothing, otherwise ask queue for data
*/
void Controller::do_action(ActionResponse *action)
{
    bool action_type = action->getSend_message();
    int queue = action->getQueue();
    int num_msg_to_send = action->getMsg_to_send();
    last_select_power_source = action->getSelect_power_source();
    
    //If action is "do nothing"
    if(!action_type){
        EV_DEBUG << "Received action " << action_type << "->Do nothing" << endl;
        do_nothing();
    }
    else{
        EV_DEBUG << "Received action " << action_type << "->Send data" << endl;
        EV_DEBUG << "Asking data to queue " << queue << " for " << num_msg_to_send << " messages" << endl;
        //Pop packet from queue
        QueueDataRequest *queueDataRequest = new QueueDataRequest();
        queueDataRequest->setData_n(num_msg_to_send);
        send(queueDataRequest, "queue_ports$o", queue); 
    }
}

void Controller::do_nothing()
{   
    EV_DEBUG << "Performing action do nothing" << endl;

    vector<float> queue_occ;
    vector<int> queue_pkt_drop_cnt;

    for(int i=0;i<num_queues;i++){
        queue_occ.push_back(queue_states[i].occupancy);
        queue_pkt_drop_cnt.push_back(queue_states[i].pkt_drop_cnt);
        EV_DEBUG << "Queue " << i << " occupancy: " << queue_occ[i] << "%" << " pkt drop count: " << queue_pkt_drop_cnt[i] << endl;
    }

    float energy_consumed=0;
    SelectPowerSource power_source = SelectPowerSource::BATTERY;

    last_reward=compute_reward(energy_consumed, power_source, queue_occ, queue_pkt_drop_cnt);
     EV_DEBUG << "Doing nothing has generated reward: " << last_reward << endl;
}

void Controller::forward_data(const DataMsg *data[], size_t num_data){

    EV_DEBUG << "Performing action send data" << endl;

    //No data to send
    if(num_data==0){
        last_reward=-1000;
    }
    else{
        //TODO Implement the effective send, for now it's only simulated by causing the effects of send like discharge
        
        //Get data needed to reward computation
        vector<float> queue_occ;
        vector<int> queue_pkt_drop_cnt;

        for(int i=0;i<num_queues;i++){
            queue_occ.push_back(queue_states[i].occupancy);
            queue_pkt_drop_cnt.push_back(queue_states[i].pkt_drop_cnt);
            EV_DEBUG << "Queue " << i << " occupancy: " << queue_occ[i] << "%" << " pkt drop count: " << queue_pkt_drop_cnt[i] << endl;
            queue_states[i].pkt_drop_cnt=0; //Reset pkt drop count after taking the value useful for reward calculation
        }
        
        mWs_t energy_consumed = power_model->calc_tx_consumption_mWs(sizeof(*data)*8, link_cap); //*8 for bits
        SelectPowerSource power_source = last_select_power_source;
        
        //Consume energy
        switch(power_source){
            case SelectPowerSource::BATTERY:
                power_sources[SelectPowerSource::BATTERY]->discharge(num_data*energy_consumed); 
                break;
            case SelectPowerSource::POWER_CHORD:
                power_sources[SelectPowerSource::POWER_CHORD]->discharge(num_data*energy_consumed);
                break;
            default:
                EV << "Error: do_action power source not recognized" << endl;
                break;
        }
        EV_DEBUG << "Consumed energy: " << energy_consumed << " mWs from power source: " << power_source << endl;
        last_reward=compute_reward(energy_consumed, power_source, queue_occ, queue_pkt_drop_cnt);
    }
    
    EV_DEBUG << "Sending message has generated reward: " << last_reward << endl;
}

reward_t Controller::compute_reward(float energy_consumed, SelectPowerSource power_source, vector<float> queue_occ, vector<int> queue_pkt_drop_cnt){

    EV_DEBUG << "Computing reward" << endl;

    // Compute reward and write it in the action request
    reward_t energy_term=power_sources[power_source]->getCostPerMWh()*energy_consumed;

    reward_t queue_term=0;
    reward_t pkt_drop_term=0;

    for(int i=0; i<num_queues; i++){
        queue_term+=queue_occ_cost[i]*queue_occ[i];
        pkt_drop_term+=pkt_drop_cost[i]*queue_pkt_drop_cnt[i];
    }

    EV_DEBUG << "Energy term: " << energy_term << endl;
    EV_DEBUG << "Queue term: " << queue_term << endl;
    EV_DEBUG << "Pkt drop term: " << pkt_drop_term << endl;

    reward_t reward=-(pkt_drop_penalty_weight*pkt_drop_term+queue_occ_penalty_weight*queue_term+energy_penalty_weight*energy_term);
    
    return reward;
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
    reward_msg.setValue(last_reward);
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
    float pkt_drop_cost_A = par("pkt_drop_cost_A").doubleValue();
    float pkt_drop_cost_B = par("pkt_drop_cost_B").doubleValue();
    float queue_occ_cost_A = par("queue_occ_cost_A").doubleValue();
    float queue_occ_cost_B = par("queue_occ_cost_B").doubleValue();

    for(int i=0; i<num_queues; i++){
        pkt_drop_cost.push_back(pkt_drop_cost_A*i+pkt_drop_cost_B);
        queue_occ_cost.push_back(queue_occ_cost_A*i+queue_occ_cost_B);
        queue_states[i].pkt_drop_cnt=0;
        queue_states[i].occupancy=0;
    }
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
    // add more module params here ...

    EV_DEBUG << "Power model tx_mW: " << power_model->getTx_mW() << endl;
    EV_DEBUG << "charge battery timeout delta: "<< charge_battery_timeout_delta << endl;
    EV_DEBUG << "ask action timeout delta: " << ask_action_timeout_delta << endl;
    EV_DEBUG << "Power model tx_mW: " << power_model->getTx_mW() << endl;
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
    EV_DEBUG << "Action response received" << endl;

    do_action(msg);
    start_timer(ask_action_timeout);

}

void Controller::handleAskActionTimeout(Timeout *msg)
{
    EV_DEBUG << "Ask action timeout expired at " << simTime() << endl;
    ask_action();
}

void Controller::handleChargeBatteryTimeout(Timeout *msg)
{
    charge_battery();
    EV_DEBUG << "battery charged at "
     << power_sources[SelectPowerSource::BATTERY]->getCharge() << endl;
        
    start_timer(charge_battery_timeout);
}

void Controller::handleQueueDataResponse(QueueDataResponse *msg)
{    
    size_t num_data_recv = msg->getDataArraySize();
    EV_DEBUG << "Queue data response size: " << num_data_recv << endl;

    //Update queue state
    size_t queue_idx; 
    queue_idx = msg->getArrivalGate()->getIndex();
    const QueueStateUpdate *stateUpdate=msg->getStateUpdate();
    update_queue_state(const_cast<QueueStateUpdate *>(stateUpdate), queue_idx);
    
    //Retrieve and forward data
    const DataMsg *data[num_data_recv];

    //Get data to send and queue state from received queue message
    for(int i=0; i<num_data_recv; i++){
        data[i]= msg->getData(i); 
    }

    forward_data(data, num_data_recv);
}

void Controller::handleQueueStateUpdate(QueueStateUpdate *msg)
{    
    size_t queue_idx; 
    queue_idx = msg->getArrivalGate()->getIndex();

    update_queue_state(msg, queue_idx);

}

//Node behaviour at message reception
void Controller::handleMessage(cMessage *msg)
{

    if (is_agentc_msg(msg)){
        EV_DEBUG << "Agent msg received by controller" << endl;
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
        EV_DEBUG << "Simulation msg received by controller" << endl;
        switch (msg->getKind())
        {
        //Add cases for other message types
        default:
            EV_ERROR << "Controller: unrecognized simulation message kind " 
            << msg->getKind() << endl;
            break;
        }
    }
    else if (is_timeout_msg(msg)){
        EV_DEBUG << "Timeout msg received by controller" << endl;
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
        EV_DEBUG << "Queue msg received by controller" << endl;
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
    delete battery_charger;
    for (PowerSource *ps : power_sources){
        delete ps;
    }
}
