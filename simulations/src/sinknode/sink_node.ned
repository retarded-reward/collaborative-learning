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

package org.cl.simulations.sinknode;
import org.cl.simulations.sinknode.SinkController;


// Sink node of the network which receives data and sends reward
module SinkNode
{
    gates:
        inout port[10];
    submodules:
        sinkController: SinkController;    
    connections allowunconnected:
        for i=0..9 {
            port[i] <--> sinkController.network_port[i];
        }
}
