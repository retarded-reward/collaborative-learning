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
#include "RewardMsg_m.h"

using namespace omnetpp;

class Controller : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
  private:
    void handleActionResponse(ActionResponse *msg); 
    void handleDataMsg(DataMsg *msg);
    void handleRewardMsg(RewardMsg *msg);
};

#endif
