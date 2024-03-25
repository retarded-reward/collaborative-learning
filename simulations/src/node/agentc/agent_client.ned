package org.cl.simulations.node.agentc;

// Bridge between the node and the agent implementation
simple AgentClient{

    parameters:
        @class(AgentClientPybind);
    
    gates:
        inout port;

}