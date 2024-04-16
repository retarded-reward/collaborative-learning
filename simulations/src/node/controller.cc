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

Define_Module(Controller);

//Node behaviour when started
void Controller::initialize()
{
    // Tests the agent client by sending it a request for an action.
    // Normal workflow should expect a response from the agent client containing
    // the action to take.

    /*NodeStateMsg *neighbour = new NodeStateMsg();
    neighbour->setEnergy(100);
    neighbour->setHas_packet_in_buffer(false);
    neighbour->setPower_state(NodePowerState::ON);

    ActionRequest *ar = new ActionRequest();

    ar->getStateForUpdate().setEnergy(100);
    ar->getStateForUpdate().setHas_packet_in_buffer(false);
    ar->getStateForUpdate().setPower_state(NodePowerState::OFF);
    ar->getStateForUpdate().appendNeighbour(*neighbour);

    ar->setRewardArraySize(1);
    ar->getRewardForUpdate(0).setMessage_id(1);
    ar->getRewardForUpdate(0).setValue(10);

    send(ar, "agent_port$o");*/
}

void Controller::handleActionResponse(ActionResponse *msg)
{
    // TODO: implement this method
    
    EV << "Action response received";

    EV  << "Change power state: " << msg->getChange_power_state() << endl;
}

void Controller::handleDataMsg(DataMsg *msg)
{
    // TODO: implement this method
    
    EV << "Data received "<< msg->getData();
    send(msg, "sink_port$o", 0);

}

void Controller::handleRewardMsg(RewardMsg *msg)
{
    //TODO implement this method
    EV << "Received reward: " << msg->getSink_reward().getValue() << " for message id: " << msg->getSink_reward().getMessage_id() << endl;
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
            << msg->getKind();
            break;
        }
    }
    else if(is_sim_msg(msg)){
        switch (msg->getKind())
        {
        case (int) SimulationMsgKind::DATA_MSG:
            handleDataMsg((DataMsg *) msg);
            break;
        case (int) SimulationMsgKind::REWARD_MSG:
            handleRewardMsg((RewardMsg *) msg);
            break;
        //Add cases for other message types
        default:
            EV_ERROR << "Controller: unrecognized simulation message kind " 
            << msg->getKind();
            break;
        }
    }

}
