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

class Controller : public cSimpleModule
{
  protected:
    reward_t last_reward; //Last reward computed

    ActionResponse *last_action_response; //Last action received

    vector<PowerSource *> power_sources;
    NICPowerModel *power_model;
       
    Timeout *ask_action_timeout;

    /**
     * The i-th element of this vector represents the up-to-date state
     * of the i-th queue.
    */
    vector<QueueState> queue_states;
    
    /**
     * Module parameters:
    */
    vector<float> pkt_drop_cost;
    vector<float> queue_occ_cost;
    float pkt_drop_penalty_weight;
    float queue_occ_penalty_weight;
    float energy_penalty_weight;
    int num_queues;
    int ask_action_timeout_delta;
    int data_buffer_capacity;
    int max_neighbours;
    float link_cap;
    cValueMap *power_models;
    cValueMap *power_source_models;


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
    void do_nothing();
    /**
     * Executes the action received from the agent client and restarts
     * ask action timer.
    */
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
     * Init methods:
     * 
     * Must be called in initialize() method
    */
    
    void init_ask_action_timer();
    void init_reward_params();
    void init_module_params();
    void init_power_model();
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
    void handleDataMsg(DataMsg *msg);
    void handleAskActionTimeout(Timeout *msg);
    void handleQueueDataResponse(QueueDataResponse *msg);
    void handleQueueStateUpdate(QueueStateUpdate *msg);
    /**Specialized handlers (END)*/

    //Util methods
    reward_t compute_reward(float energy_consumed, SelectPowerSource power_source, vector<float> queue_occ, vector<int> queue_pkt_drop_cnt);
    /**
     * Updates tracked state of corresponing queue
    */
    void update_queue_state(QueueStateUpdate *msg, size_t queue_idx);

};

#endif
