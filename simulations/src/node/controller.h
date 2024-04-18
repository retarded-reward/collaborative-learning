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

using namespace omnetpp;
using namespace std;

class Controller : public cSimpleModule
{
  protected:

    void send_test_action_request(); 
   
    Timeout *ask_action_timeout;
    /**delta timeout time in ms*/
    int ask_action_timeout_delta;
    
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
    
    void start_timer(Timeout *timeout);
    void stop_timer(Timeout *timeout);
    
    /**
     * Init methods:
     * 
     * Must be called in initialize() method
    */
    
    void init_ask_action_timer();
    /**must be called before all other inits*/
    void init_module_params();

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
};

#endif
