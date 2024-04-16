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

#include "sink_controller.h"
#include "SimulationMsg_m.h"
#include "DataMsg_m.h"
#include "RewardMsg_m.h"

Define_Module(SinkController);

void SinkController::initialize()
{
    last_message_id = -1;
}


void SinkController::handleMessage(cMessage *msg)
{ 
    if(is_sim_msg(msg)){
        switch (msg->getKind())
        {
        case (int) SimulationMsgKind::DATA_MSG:
            handleDataMsg((DataMsg *) msg);
            break;
        //Add cases for other message types
        default:
            EV_ERROR << "Controller: unrecognized simulation message kind " 
            << msg->getKind();
            break;
        }
    }

    delete msg;
}

void SinkController::sendReward(int message_id)
{   
    int n = gateSize("network_port");
    for(int i = 0; i < n; i++){
        RewardMsg *reward = new RewardMsg();
        SinkRewardMsg *sink_reward = new SinkRewardMsg();
        sink_reward->setMessage_id(message_id);
        sink_reward->setValue(5);    //TODO Choose appropriate reward
        reward->setSink_reward(*sink_reward);
        send(reward, "network_port$o", i);  
    }
}

void SinkController::handleDataMsg(DataMsg *msg){
    int message_id = msg->getMsg_id();
    if(message_id > last_message_id){
        last_message_id = message_id;
        sendReward(message_id);
    }
}