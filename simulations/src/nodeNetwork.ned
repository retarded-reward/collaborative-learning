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
    submodules:
        node[number_of_nodes]: Node;
        srcNode: SrcNode;
    connections allowunconnected:
        srcNode.port[0] <--> node[0].port[0]; 
              
}
