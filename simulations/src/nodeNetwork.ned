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

package org.cl.simulations;
import org.cl.simulations.node.Node;
import org.cl.simulations.srcnode.SrcNode;
import org.cl.simulations.sinknode.SinkNode;

//Network description including nodes and their connections
network NodeNetwork
{	
    parameters:
        int number_of_nodes @value(number_of_nodes);//= default(1);
        int number_of_queues @value(number_of_queues);
    submodules:
        node[number_of_nodes]: Node;
        srcNode[number_of_queues]: SrcNode;
    connections allowunconnected:
        for i=0..number_of_queues-1 {
            srcNode[i].data_out[0] --> node[0].queue_ports[i];
        }
              
}
