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

#include "node.h"

Define_Module(Node);

//Node behaviour when started
void Node::initialize()
{
    cMessage *information = new cMessage();
    send(information, "port$o");
}

//Node behaviour at message reception
void Node::handleMessage(cMessage *msg)
{
    EV << "Message received";
    cMessage *answer = new cMessage();
    sendDelayed(answer, uniform(10, 60), "port$o");

}
