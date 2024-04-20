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
#include "fc_cqueue.h"
#include "power/power_source.h"
#include <vector>

using namespace omnetpp;
using namespace std;

struct NeighbourState
{
  NodeStateMsg *state;
  /**
   * if false, we do not know if the neighbour is reachable. This entry can be replaced
   * by a new one of a reachable neighbour.
  */
  bool reachable;
};

class Controller : public cSimpleModule
{
  protected:

    /**
     * A node needs energy to execute actions.
    */
    PowerSource *power_source;
    /**
     * Buffer where the nodes stores the data messages that it receives
     * before forwarding them.
    */
    FixedCapCQueue *data_buffer;
    /**
     * The power state of the node.
    */
    NodePowerState power_state;

    /**
     * Neighbours state informations 
    */
    vector<NeighbourState> neighbours;

    // TODO: implement neighbours state
       
    Timeout *ask_action_timeout;
    
    /**
     * Module parameters:
    */

    int id;
    int ask_action_timeout_delta;
    double battery_capacity;
    int data_buffer_capacity;
    int max_neighbours;
    float link_cap;
    /* Module parameters (END)*/
    
    /**
     * Action Event Flow:
     * 
     * When to ask/do actions?
     * 
     * data received    => stop timer, ask action
     * action received  => do action, start timer
     * timer timeout    => ask action 
    */
    
    void ask_action();
    void do_action(ActionResponse *action);
    /**Action Event Flow (END)*/
    
    void start_timer(Timeout *timeout);
    void stop_timer(Timeout *timeout);

    void sample_state(NodeStateMsg &state_msg);
    
    /**
     * Init methods:
     * 
     * Must be called in initialize() method
    */
    
    void init_ask_action_timer();
    void init_module_params();
    void init_power_source();
    void init_data_buffer();
    void init_neighbours();
    void init_power_state();
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
    void handleRewardMsg(RewardMsg *msg);
    /**Specialized handlers (END)*/

    /* test methods*/
    void send_test_action_request();
    /* test methods (END)*/

};

#endif
