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

Define_Module(Controller);

//Node behaviour when started
void Controller::initialize()
{
    //cMessage *information = new cMessage();
    //send(information, "network_port$o");
    
    // Tests the agent client by sending it a request for an action.
    // Normal workflow should expect a response from the agent client containing
    // the action to take.

    NodeStateMsg *neighbour = new NodeStateMsg();
    neighbour->setEnergy(100);
    neighbour->setHas_packet_in_buffer(false);
    neighbour->setPower_state(NodePowerState::ON);

    ActionRequest *ar = new ActionRequest();
    ar->setName(AgentClient::MSG_TOPIC.c_str());
    ar->setKind((int) AgentClientMsgKind::ACTION_REQUEST);

    ar->getStateForUpdate().setEnergy(100);
    ar->getStateForUpdate().setHas_packet_in_buffer(false);
    ar->getStateForUpdate().setPower_state(NodePowerState::OFF);
    ar->getStateForUpdate().appendNeighbours(*neighbour);

    ar->setRewardArraySize(1);
    ar->getRewardForUpdate(0).setMessage_id(1);
    ar->getRewardForUpdate(0).setValue(10);

    send(ar, "agent_port$o");
}

//Node behaviour at message reception
void Controller::handleMessage(cMessage *msg)
{
    EV << "Message received";
    //send(new cMessage(), "agent_port$o");
    //cMessage *answer = new cMessage();
    //sendDelayed(answer, uniform(10, 60), "network_port$o");

}
