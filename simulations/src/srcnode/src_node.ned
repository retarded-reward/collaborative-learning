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

package org.cl.simulations.srcnode;
import org.cl.simulations.srcnode.SrcController;


// a node of the network that generates data
module SrcNode
{
    gates:
        inout port[];
    submodules:
        srcController: SrcController;    
    connections allowunconnected:
        port++ <--> srcController.network_port++;
}
