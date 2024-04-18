package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    parameters:
        int ask_action_timeout_delta @unit(ms); // timeout delta for asking action (in sim time)
    
    gates:
        inout network_port[];
        inout agent_port;   
}
