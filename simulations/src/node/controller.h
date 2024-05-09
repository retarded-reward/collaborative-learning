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

#ifndef __DEMO_NODE_H_
#define __DEMO_NODE_H_

#include <omnetpp.h>
#include "ActionResponse_m.h"
#include "DataMsg_m.h"
#include "Timeout_m.h"
#include "RewardMsg_m.h"
#include "NodeStateMsg_m.h"
#include "power/power_source.h"
#include "power/nic_power_model.h"
#include <vector>
#include "QueueDataResponse_m.h"
#include "units.h"

using namespace omnetpp;
using namespace std;

struct QueueState {
  percentage_t occupancy;
  int pkt_drop_cnt;
};

/**
 * Reward can be computed as a combination of different terms.
 * Each term comes with a signal along with its weight.
 * 
 * The signal is a function of a measured quantity which is informative about the
 * agent perfomance (e.g. energy consumption, queue occupancy, etc.).
 * 
 * The weight is a scalar that determines the importance of the corresponding term in
 * the reward computation.
 * 
 * Since signals are dynamic expressions (specified in .ini file), their symbols must
 * be bound to actual values at runtime before computing the term value.
*/
class RewardTerm {
  
protected:
  reward_t weight;
  cOwnedDynamicExpression *signal;

public:
  RewardTerm(reward_t weight, cOwnedDynamicExpression *signal)
   : weight(weight), signal(signal) {}
  RewardTerm(cValueMap *reward_term_map)
   : RewardTerm(reward_term_map->get("weight").doubleValue(),
    (cOwnedDynamicExpression *)reward_term_map->get("signal").objectValue()) {}
  RewardTerm(cValueMap *reward_term_models, const char *reward_term_model_name)
   : RewardTerm((cValueMap *) reward_term_models->get(reward_term_model_name)
    .objectValue()) {}

  void bind_symbols(map<string, cValue> symbols){
    // resolver is owned by the dynamic expression, no need to delete it
    // before setting a new one
    signal->setResolver(new cDynamicExpression::SymbolTable(symbols));
  }

  reward_t compute() {
    if (signal->getResolver() == nullptr)
      throw cRuntimeError("Signal resolver is not set. Call bind_symbols() first.");
    return weight * signal->doubleValue();
  }

  reward_t getWeight() const {
    return weight;
  }

  cOwnedDynamicExpression* getSignal() const {
    return signal;
  }

};
class Controller : public cSimpleModule
{
  protected:
    reward_t last_reward; //Last reward computed

    SelectPowerSource last_select_power_source = (SelectPowerSource)0; //Last power source selected, needed cause data retrieving for queue is asynchroneous and so the "forward" action is splitted in 2 parts
    vector<mWh_t> last_energy_consumed; //Last energy consumed for every power source
    percentage_t last_charge_rate = 0;

    vector<PowerSource *> power_sources;
    NICPowerModel *power_model;
    PowerSource *battery_charger;
       
    Timeout *ask_action_timeout;
    Timeout *charge_battery_timeout;

    /**
     * The i-th element of this vector represents the up-to-date state
     * of the i-th queue.
    */
    vector<QueueState> queue_states;
    
    /**
     * Module parameters:
    */
    int num_queues;
    s_t ask_action_timeout_delta;
    int data_buffer_capacity;
    int max_neighbours;
    Mbps_t link_cap;
    s_t charge_battery_timeout_delta;
    cValueMap *power_models;
    cValueMap *power_source_models;
    cValueMap *reward_term_models;

    /* Module parameters (END)*/
    
    /**
     * Action Event Flow:
     * 
     * When to ask/do actions?
     * 
     * action received  => do action, start timer
     * timer timeout    => ask action 
    */
    
    /**
     * Interrogates agent client to get the next action to be executed.
    */
    void ask_action();
    /**
     * Perform action
    */
    void do_action(ActionResponse *action);    
    void forward_data(const DataMsg *data[], size_t num_data);
    void _forward_data(const DataMsg *data[], size_t num_data);
    void do_nothing();
    /**Action Event Flow (END)*/
    
    void start_timer(Timeout *timeout);
    void stop_timer(Timeout *timeout);

    /**
     * Samples state and writes it in the NodeStateMsg object
    */
    void sample_state(NodeStateMsg &state_msg);
    void sample_power_sources(NodeStateMsg &state_msg);
    void sample_queue_states(NodeStateMsg &state_msg);

    void sample_reward(RewardMsg &reward_msg);

    /**
     * Measures values for all quantities managed by
     * the controller needed to compute statistics.
     * 
     * Since most statistics will be used to print charts, it is important to sample
     * all quantities at the same time in order to make confrontations easier.
    */
    void measure_quantities();
    
    /**
     * Init methods:
     * 
     * Must be called in initialize() method
    */
    
    void init_timers();
    void init_reward_params();
    void init_module_params();
    void init_power_sources();
    void init_queue_states();    
    /** Init methods (END)*/

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    ~Controller();

    /**
     * Specialized handlers:
     * 
     * Must be called inside handleMessage() method to handle messages of
     * specific kinds.
    */
    
    void handleActionResponse(ActionResponse *msg); 
    void handleAskActionTimeout(Timeout *msg);
    void handleChargeBatteryTimeout(Timeout *msg);
    void handleQueueDataResponse(QueueDataResponse *msg);
    void handleQueueStateUpdate(QueueStateUpdate *msg);
    /**Specialized handlers (END)*/

    //Util methods
    reward_t compute_reward();
    /**
     * Creates a RewardTerm object from a reward term model and adds it to the
     * user provided vector of reward terms.
     * The symbols used in the RewardTerm signal are bound to the values provided
     * in the symbols map.
    */
    void include_reward_term(const char* reward_term_model_name,
    map<string, cValue> symbols, vector<RewardTerm *> &reward_terms);
    /**
     * Updates tracked state of corresponing queue
    */
    void update_queue_state(QueueStateUpdate *msg, size_t queue_idx);
    void charge_battery();
};

#endif
