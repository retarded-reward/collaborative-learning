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

// a node of the network
module Node
{
    gates:
        inout port;
    submodules:
        controller: Controller;
        agent: AgentClient;
    connections:
        port <--> controller.network_port;
        agent.port <--> IdealChannel <--> controller.agent_port;
        
    
}
