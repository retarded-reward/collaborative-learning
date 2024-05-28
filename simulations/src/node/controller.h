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

#define set_if_greater(_actual, _candidate) if (_candidate > _actual) _actual = _candidate  

struct QueueState {
  percentage_t occupancy;
  int pkt_drop_cnt;
  int pkt_inbound_cnt;
  int max_pkt_drop_cnt;

  void reset_counts(){
    pkt_drop_cnt = 0;
    pkt_inbound_cnt = 0;
  }
};

/**
 * Normalizer is an abstract class that defines a method to normalize a value.
 * 
 * Normalization is the process of scaling and centering variables.
 * This can be useful when the variables have different units or scales.
 * 
 * For example, if we have two variables, one in the range [0, 100] and the other
 * in the range [0, 1000], the second variable will have a greater impact on the
 * computation of the reward.
 * 
 * Normalization can be used to scale the variables to the same range, so that
 * they have the same impact on the computation of the reward.
*/
class Normalizer {

  public:
    virtual double normalize(double value) { return value; };
};

/**
 * Restricts values from range [min, max] to [a, b]
 * https://en.m.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)
 * 
 * For example, if we want to transform values in percentages:
 * 
 * Normalizer *normalizer = new MinMaxNormalizer(0, max, 0, 1);
 * double percentage = normalizer->normalize(value) * 100;
*/
class MinMaxNormalizer : public Normalizer {

  private:
    double min;
    double max;
    double a;
    double b;

  public:
    MinMaxNormalizer(double min, double max) : MinMaxNormalizer(min, max, -1, 0) {}
    MinMaxNormalizer(double min, double max, double a, double b)
     : min(min), max(max), a(a), b(b) {}

    double normalize(double value) override {
      if (max == min) return 0;
      return ((value - min) * (b - a)) / (max - min);
    }
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
  Normalizer *normalizer;

  bool cached = false;
  reward_t cached_value;

public:
  RewardTerm(reward_t weight, cOwnedDynamicExpression *signal)
   : weight(weight), signal(signal->dup()) {
    normalizer = new Normalizer();
   }
  RewardTerm(cValueMap *reward_term_map)
   : RewardTerm(reward_term_map->get("weight").doubleValue(),
    (cOwnedDynamicExpression *)reward_term_map->get("signal").objectValue()) {}
  RewardTerm(cValueMap *reward_term_models, const char *reward_term_model_name)
   : RewardTerm((cValueMap *) reward_term_models->get(reward_term_model_name)
    .objectValue()) {}
  
  ~RewardTerm(){
    delete signal;
    delete normalizer;
  }

  reward_t getWeight() const {
    return weight;
  }

  cOwnedDynamicExpression* getSignal() const {
    return signal;
  }

  RewardTerm *setWeight(reward_t weight) {
    this->weight = weight;
    return this;
  }

  RewardTerm *setNormalizer(Normalizer *normalizer) {
    delete this->normalizer;
    this->normalizer = normalizer;
    return this;
  }
  
  RewardTerm *bind_symbols(map<string, cValue> symbols){
    // resolver is owned by the dynamic expression, no need to delete it
    // before setting a new one
    signal->setResolver(new cDynamicExpression::SymbolTable(symbols));
    return this;
  }

  reward_t compute(bool use_cache = true) {
    
    reward_t normalized_value;
    
    if (!use_cache || !cached){
      if (signal->getResolver() == nullptr)
        throw cRuntimeError("Signal resolver is not set. Call bind_symbols() first.");
      
      normalized_value = normalizer->normalize(signal->doubleValue());
      cached_value = weight * normalized_value;
    }  

    return cached_value;
  }

  void invalidate_cache(){
    cached = false;
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
    PowerSource *most_expensive_power_source = nullptr;
       
    Timeout *ask_action_timeout;
    Timeout *charge_battery_timeout;

    B_t max_packet_size = 0;
    vector<mWh_t> max_energy_consumed;
    
    /**
     * The i-th element of this vector represents the up-to-date state
     * of the i-th queue.
    */
    vector<QueueState> queue_states;

    int sum_priorities;
    reward_t sum_power_sources_costs;
    
    /**
     * Module parameters:
    */
    int num_queues;
    s_t ask_action_timeout_delta;
    int max_neighbours;
    Mbps_t link_cap;
    s_t charge_battery_timeout_delta;
    cValueMap *power_models;
    cValueMap *power_source_models;
    cValueMap *reward_term_models;
    reward_t hybris;

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
     * Measures values for some quantities managed by
     * the controller needed to compute statistics.
     * 
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
     * Values computed by the signal are normalized using the provided normalizer.
    */
    void include_reward_term(const char* reward_term_model_name,
    map<string, cValue> symbols, vector<RewardTerm *> &reward_terms, 
     Normalizer *normalizer);
    /**
     * Includes a term in the reward computation without normalization.
    */
    void include_reward_term(const char* reward_term_model_name,
     map<string, cValue> symbols, vector<RewardTerm *> &reward_terms);
    
    inline reward_t illegal_action_penalty()
    {
      return hybris;
    }

    /**
     * Updates tracked state of corresponing queue
    */
    void update_queue_state(QueueStateUpdate *msg, size_t queue_idx);
    void charge_battery();
    inline void measure_action(bool must_send, unsigned int queue, unsigned int power_source);
};

#endif
