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

package org.cl.simulations.node;

import ned.IdealChannel;
import org.cl.simulations.node.agentc.AgentClient;
import org.cl.simulations.node.Controller;
import org.cl.simulations.node.queue.Queue;

// a node of the network
module Node
{
    parameters:
        int number_of_ports  @value(number-of-ports);//=default(1);
        int num_queues;
    gates:
        output network_out[number_of_ports] @loose;
        input queue_ports[num_queues] @loose;
    submodules:
        controller: Controller;
        agent: AgentClient;
        queues[num_queues]: Queue;
    connections allowunconnected:
        agent.port <--> IdealChannel <--> controller.agent_port; 
        for i=0..number_of_ports-1 {
            network_out[i] <-- controller.network_port[i];
        }
        for i=0..num_queues-1 {
            controller.queue_ports++ <--> queues[i].servers_inout++;
            queue_ports[i] --> queues[i].data_in;
        }
}
