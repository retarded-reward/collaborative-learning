package org.cl.simulations.node.agentc;

// Bridge between the node and the agent implementation
simple AgentClient{

    parameters:
        @class(AgentClientPybind);
        @display("i=device/cpu");
    
        int num_of_queues;
        string implementation;
    gates:
        inout port;

}