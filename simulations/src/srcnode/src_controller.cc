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

#include "src_controller.h"
#include "SimulationMsg_m.h"
#include "DataMsg_m.h"
#include <cmath>

Define_Module(SrcController);

//Node behaviour when started
void SrcController::initialize()
{
    message_count = 0;
    //Send message to node itself cause can't enter loop in initialize
    schedule_data();

}

void SrcController::handleMessage(cMessage *msg)
{
    // Handle incoming messages
    if (msg->isSelfMessage()) {
        sendData();
        schedule_data();
    }
    delete msg;
    
}


int SrcController::randomIntGenerator(int min, int max)
{
    int res = min + rand() % (max - min + 1);
    return res;
}

void SrcController::schedule_data()
{
    simtime_t delay = par("send_interval").doubleValue();
    scheduleAt(simTime()+delay, new cMessage());
}


//Send random data with exponential distribution of mean 12 at a random neighbour
void SrcController::sendData()
{   
    int n = gateSize("network_port");
    int neigh = randomIntGenerator(0, n-1);
    DataMsg *data = new DataMsg();
    float data_size = ceil(par("pkt_size").doubleValue());
    EV_DEBUG << "Sending data of size " << data_size << " to node " << neigh << "\n";
    data->setData(data_size);
    message_count++;
    send(data, "network_port", neigh);
}
