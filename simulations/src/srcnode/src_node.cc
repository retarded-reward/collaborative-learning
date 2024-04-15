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

#include "src_node.h"
#include "SimulationMsg_m.h"
#include "DataMsg_m.h"

Define_Module(SrcNode);

//Node behaviour when started
void SrcNode::initialize()
{
    //Send message to node itself cause can't enter loop in initialize
    scheduleAt(simTime(), new cMessage());	
}

void SrcNode::handleMessage(cMessage *msg)
{
    // Handle incoming messages
    if (msg->isSelfMessage()) {
        sendData();
    }
    delete msg;
    
}

float SrcNode::randomDataGenerator(float max)
{
    float data = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/max));
    return data;
}


//Send random data with exponential distribution of mean 12
void SrcNode::sendData()
{   
    int n = gateSize("port");
    message_count++;
    DataMsg *data = new DataMsg();
    data->setMsg_id(message_count);
    data->setData(randomDataGenerator(200.0));
    send(data, "port$o", n-1);
    scheduleAt(simTime()+exponential(12), new cMessage());
}
