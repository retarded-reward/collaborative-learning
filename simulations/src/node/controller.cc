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
#include "statistics.h"

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
    sample_reward(ar->getRewardForUpdate());

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
    queue_states[queue_idx].pkt_inbound_cnt += msg->getNum_of_inbound();
    set_if_greater(queue_states[queue_idx].max_pkt_drop_cnt, queue_states[queue_idx].pkt_drop_cnt);
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

inline void Controller::measure_action(bool must_send, unsigned int queue, unsigned int power_source)
{
    unsigned int action;

    if(!must_send) 
        measure_quantity("do_nothing", 1);
    else if (power_source == SelectPowerSource::BATTERY)
        measure_quantity("send_battery", 1);
    else if (power_source == SelectPowerSource::POWER_CHORD)
        measure_quantity("send_powerchord", 1);

    if (must_send)
    {
        action = queue * power_sources.size() + power_source + 1;
    }
    else 
    {
        action = 0;
    }
        
    measure_quantity("action", action);
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

    measure_action(action_type, queue, last_select_power_source);    
    
    //If action is "do nothing"
    if(!action_type){
        EV_DEBUG << "Received action " << action_type << "->Do nothing" << endl;
        do_nothing();
        measure_quantities();
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

    last_energy_consumed[SelectPowerSource::POWER_CHORD]=0;
    last_energy_consumed[SelectPowerSource::BATTERY]=0;

    last_reward=compute_reward();
     EV_DEBUG << "Doing nothing has generated reward: " << last_reward << endl;
}

void Controller::forward_data(const DataMsg *data[], size_t num_data){
    
    EV_DEBUG << "Performing action send data" << endl;

    s_t service_interval;
    b_t data_bits;

    //No data to send
    if(num_data==0){
        // wait, that's illegal
        last_reward=illegal_action_penalty();
    }
    else{
        _forward_data(data, num_data);

        // calcs service interval
        for (int i = 0; i < num_data; i++){
            data_bits = data[i]->getData() * 8;
            service_interval = data_bits * 1e-6 / link_cap;
            measure_quantity("service_interval", service_interval);
            measure_quantity("response_time", simTime() - data[i]->getQueueing_time() + service_interval);
        }
        if (service_interval > 0)
            measure_quantity("service_interval", service_interval);
        
        
        last_reward=compute_reward();
    }

    measure_quantities();
    
    EV_DEBUG << "Sending message has generated reward: " << last_reward << endl;
}

//TODO Implement the effective send, for now it's only simulated by causing the effects of send like discharge
void Controller::_forward_data(const DataMsg *data[], size_t num_data)
{
    //Compute consumed energy
    last_energy_consumed[SelectPowerSource::POWER_CHORD]=0;
    last_energy_consumed[SelectPowerSource::BATTERY]=0;
    mWh_t tot_consumed=0;
    mWh_t battery_level;

    for(int i=0; i<num_data; i++){
        EV_DEBUG << "Data " << i << " size: " << (int) data[i]->getData() << std::endl;
        tot_consumed
            += (60 * 60 * power_model-> calc_tx_consumption_mWs((int) data[i]->getData()*8, link_cap)); // *8 for bits, converted in mWh
    }      
    
    //Consume energy
    switch(last_select_power_source){

        case SelectPowerSource::BATTERY:
            battery_level = power_sources[SelectPowerSource::BATTERY]->getCharge();
            //If battery is not enough fallback on power chord
            if(battery_level<tot_consumed){
                last_energy_consumed[SelectPowerSource::BATTERY]=battery_level;
                last_energy_consumed[SelectPowerSource::POWER_CHORD]=tot_consumed-battery_level;
            }
            else{
                last_energy_consumed[SelectPowerSource::BATTERY]=tot_consumed;  
            }         
            break;

        case SelectPowerSource::POWER_CHORD:
            last_energy_consumed[SelectPowerSource::POWER_CHORD]=tot_consumed;
            break;

        default:
            EV << "Error: do_action power source not recognized" << endl;
            break;
    }

    //Discharge energy
    power_sources[SelectPowerSource::POWER_CHORD]->discharge(last_energy_consumed[SelectPowerSource::POWER_CHORD]); 
    power_sources[SelectPowerSource::BATTERY]->discharge(last_energy_consumed[SelectPowerSource::BATTERY]);
    
    for(int i=0; i<power_sources.size(); i++){
        EV_DEBUG << "Consumed energy: " << last_energy_consumed[i] << " mWh from power source: " << i << endl;
    }
}

reward_t Controller::compute_reward(){

    EV_DEBUG << "Computing reward" << endl;

    reward_t reward = 0;
    vector<RewardTerm *> reward_terms;

    double energy_penalty_norm_factor; 
    double pkt_drop_penalty_norm_factor;
    double queue_occ_penalty_norm_factor;

    /**
     * To normalize the reward terms, we need to know the maximum value
     * for each term. To compute it, we must leverage the same signal used by the
     * corresponding reward term. So we need to use a temporary reward term with
     * a signal value corresponding to the maximum possible value of it.
    */

    // includes energy term
    for(int i=0; i<power_sources.size(); i++)
    {
        set_if_greater(max_energy_consumed[i], last_energy_consumed[i]);
        energy_penalty_norm_factor
            = RewardTerm(reward_term_models, "energy_penalty").bind_symbols(
            {
                {"energy_consumed", cValue(max_energy_consumed[i])},
                {"cost_per_mWh", cValue(sum_power_sources_costs)}
            })->setWeight(1)->compute();
        include_reward_term("energy_penalty",
         {
            {"energy_consumed", cValue(last_energy_consumed[i])},
            {"cost_per_mWh", (power_sources[i]->getCostPerMWh())}
         },
         reward_terms,
         new MinMaxNormalizer(0, absolute(energy_penalty_norm_factor)));
    EV_DEBUG << "Energy term for power source " << i << "included" << endl;
    EV_DEBUG << "energy_penalty_norm_factor for power source " << i << ": " << energy_penalty_norm_factor << endl;
    }
    
    // includes queue occ penalties, one term for each priority
    for (int queue = 0; queue < num_queues; queue ++)
    {
        queue_occ_penalty_norm_factor
         = RewardTerm(reward_term_models, "queue_occ_penalty").bind_symbols(
         {
            {"priority", cValue(sum_priorities)},
            {"queue_occ", cValue(100)}, //100 cause it's the max value, used for normalization
         })->setWeight(1)->compute();
        include_reward_term("queue_occ_penalty",
         {
            {"priority", cValue(queue + 1)},
            {"queue_occ", cValue(queue_states[queue].occupancy)}
         },
         reward_terms,
         new MinMaxNormalizer(0, absolute(queue_occ_penalty_norm_factor)));
        
        EV_DEBUG << "Queue occ term for priority "
         << queue << "included" << endl;
        EV_DEBUG << "max queue occ penalty for priority " << queue << ": "
         << queue_occ_penalty_norm_factor << endl;
    }

    // includes pkt drop penalties, one term for each priority
    for (int queue = 0; queue < num_queues; queue ++)
    {
        EV_DEBUG << "pkt drop count for priority " << queue << ": "
         << queue_states[queue].pkt_drop_cnt << endl;
                
        pkt_drop_penalty_norm_factor
         = RewardTerm(reward_term_models, "pkt_drop_penalty").bind_symbols(
         {
            {"priority", cValue(sum_priorities)},
            {"pkt_drop_count", cValue(queue_states[queue].pkt_inbound_cnt)}
         })->setWeight(1)->compute();
        include_reward_term("pkt_drop_penalty",
         {
            {"priority", cValue(queue + 1)},
            {"pkt_drop_count", cValue(queue_states[queue].pkt_drop_cnt)}
         },
         reward_terms,
         new MinMaxNormalizer(0, absolute(pkt_drop_penalty_norm_factor)));
        // resets pkt counts after reading them
        queue_states[queue].reset_counts();

        EV_DEBUG << "Pkt drop term for priority " 
         << queue << "included: " << endl;
        EV_DEBUG << "max pkt drop penalty for priority " << queue << ": "
         << pkt_drop_penalty_norm_factor << endl;
    }

    // computes reward by consuming and reducing all included reward terms
    for (RewardTerm *reward_term : reward_terms)
    {
        EV_DEBUG << "reward term: " << reward_term->compute() << endl;
        reward = reward + reward_term->compute();
        EV_DEBUG << "partial reward: " << reward << endl;
        delete reward_term;
    }

    if (reward < -1) EV_WARN << "reward is < -1" << endl;
    
    return reward;
}

void Controller::include_reward_term(const char* reward_term_model_name,
 map<string, cValue> symbols, vector<RewardTerm *> &reward_terms)
{
    RewardTerm *reward_term;
    
    reward_term = new RewardTerm(reward_term_models, reward_term_model_name);
    reward_term->bind_symbols(symbols);
    reward_terms.push_back(reward_term);
}

void Controller::include_reward_term(const char *reward_term_model_name,
 map<string, cValue> symbols, vector<RewardTerm *> &reward_terms, Normalizer *normalizer)
{
    include_reward_term(reward_term_model_name, symbols, reward_terms);
    reward_terms.back()->setNormalizer(normalizer);
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

void Controller::measure_quantities()
{
    mWh_t energy_consumption = 0;
    reward_t energy_expense = 0;
    percentage_t energy_potential_expense = 1;
    PowerSource *most_expensive_power_source = power_sources[0];

    // find most expensive power source
    for (PowerSource *power_source : power_sources){
        if (power_source->getCostPerMWh() > most_expensive_power_source->getCostPerMWh()){
            most_expensive_power_source = power_source;
        }
    }

    for (int i = 0; i < power_sources.size(); i++){
        energy_consumption += last_energy_consumed[i];
        energy_expense += last_energy_consumed[i] * power_sources[i]->getCostPerMWh();
        energy_potential_expense += last_energy_consumed[i] * most_expensive_power_source->getCostPerMWh();
    }
    measure_quantity("energy_expense", energy_expense);
    measure_quantity("energy_consumption", energy_consumption);
    measure_quantity("energy_potential_expense", energy_potential_expense);
    
    measure_quantity("battery_charge_level",
     power_sources[SelectPowerSource::BATTERY]->getCharge());
    measure_quantity("reward", last_reward);

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
    // reward params are now written in the reward_term_models cValueMap.
    // here are inited only the params not listed in the reward_term_models.

    sum_priorities = ((num_queues * (num_queues + 1))/2);
    EV_DEBUG << "sum_priorities: " << sum_priorities << endl;
    sum_power_sources_costs = [this](){
        reward_t sum = 0;
        for (PowerSource *ps : power_sources){
            sum += ps->getCostPerMWh();
        }
        return sum;
    }();
}


void Controller::init_module_params()
{   
    ask_action_timeout_delta = par("ask_action_timeout_delta").doubleValue();
    max_neighbours = par("max_neighbours").intValue();
    link_cap = par("link_cap").doubleValueInUnit("bps");
    power_model = new NICPowerModel();
    power_models = (cValueMap *) par("power_models").objectValue()->dup();
    cValueMap *raw_power_model = (cValueMap *) power_models->get("intel_dualband_wireless_AC_7256").objectValue();
    //Parse power model parameters
    power_model->setTx_mW(raw_power_model->get("tx_mW").doubleValueInUnit("mW"));
    power_model->setIdle_mW(raw_power_model->get("idle_mW").doubleValueInUnit("mW"));
    power_source_models = (cValueMap *) par("power_source_models").objectValue()->dup();
    num_queues = getParentModule()->par("num_queues").intValue();
    charge_battery_timeout_delta 
     = par("charge_battery_timeout_delta").doubleValue();
    reward_term_models = (cValueMap *) par("reward_term_models").objectValue()->dup();
    hybris = par("hybris").doubleValue();
    max_pkt_size = par("max_pkt_size").doubleValueInUnit("B");
    // add more module params here ...

    EV_DEBUG << "Power model tx_mW: " << power_model->getTx_mW() << "mW" <<endl;
    EV_DEBUG << "Power model idle_mW: " << power_model->getIdle_mW() << "mW" << endl;
    EV_DEBUG << "charge battery timeout delta: "<< charge_battery_timeout_delta << endl;
    EV_DEBUG << "ask action timeout delta: " << ask_action_timeout_delta << endl;
    EV_DEBUG << "hybris: " << hybris << endl;
    EV_DEBUG << "num_queues: " << num_queues << endl;
    EV_DEBUG << "max_neighbours: " << max_neighbours << endl;
    EV_DEBUG << "link_cap: " << link_cap << "bps" << endl;
    EV_DEBUG << "max_pkt_size: " << max_pkt_size << "B" << endl;
}

void Controller::init_power_sources()
{
   
    cValueMap *battery_params
     = (cValueMap *) power_source_models->get("belkin_BPB001_powerbank").objectValue();
    cValueMap *power_chord_params
     = (cValueMap *) power_source_models->get("power_chord_standard").objectValue();
    cValueMap *battery_charger_params
     = (cValueMap *) power_source_models->get("solar_panel").objectValue();
        
    
    power_sources.insert(power_sources.begin() + SelectPowerSource::BATTERY,
     new Battery(battery_params->get("cap_mWh").doubleValueInUnit("mWh")));
    power_sources[SelectPowerSource::BATTERY]
     ->setCostPerMWh(battery_params->get("cost_per_mWh").doubleValue());
    power_sources[SelectPowerSource::BATTERY]->plug();
    EV_DEBUG << "Battery capacity: " << power_sources[SelectPowerSource::BATTERY]->getCharge() << endl;
    EV_DEBUG << "Battery cost per mWh: " << power_sources[SelectPowerSource::BATTERY]->getCostPerMWh() << endl;

    power_sources.insert(power_sources.begin() + SelectPowerSource::POWER_CHORD, new PowerChord());
    power_sources[SelectPowerSource::POWER_CHORD]
     ->setCostPerMWh(power_chord_params->get("cost_per_mWh").doubleValue());
    power_sources[SelectPowerSource::POWER_CHORD]->plug();
    EV_DEBUG << "Power chord cost per mWh: " << power_sources[SelectPowerSource::POWER_CHORD]->getCostPerMWh() << endl;

    // init last energy consumed
    for(int i=0; i<power_sources.size(); i++){
        last_energy_consumed.push_back(0);
    }

    // init battery charger
    cPar &charge_rate_distribution = par("battery_charge_rate_distribution");
    battery_charger = new RandomCharger(par("battery_charge_rate_distribution"),
     battery_charger_params->get("cap_mWh").doubleValueInUnit("mWh"));
    EV_DEBUG << "max charge is " << battery_charger->getCapacity() << endl;
    battery_charger->plug();

    // inits max energy consumed    
    max_energy_consumed.resize(power_sources.size(), max_packet_size * 8 * power_model->getTx_mW());

    // find most expensive power source
    for (PowerSource *ps : power_sources){
        if(!most_expensive_power_source || ps->getCostPerMWh() > most_expensive_power_source->getCostPerMWh()){
            most_expensive_power_source = ps;
        }    
    }
        
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

    // updates max size of packets seen
    for (int i = 0; i < msg->getDataArraySize(); i++){
        if (msg->getData(i)->getData() > max_packet_size)
            max_packet_size = msg->getData(i)->getData();
    }
    
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
    delete reward_term_models;
}
