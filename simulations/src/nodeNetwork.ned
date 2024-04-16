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
    submodules:
        node[11]: Node;
        srcNode: SrcNode;
        sinkNode: SinkNode;
    connections allowunconnected:
    	for i=0..9 {
            sinkNode.port[i] <--> node[i].sink_port; 
            
            // TODO Change with proper connection criteria based on distance
            node[i].port[i] <--> node[i+1].port[i];
            srcNode.port[i] <--> node[i].src_port; 
                 
        }
              
}
