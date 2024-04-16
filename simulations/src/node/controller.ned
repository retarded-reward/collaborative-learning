package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    gates:
        inout network_port[10];
        inout src_port[1];
        inout sink_port[1];
        inout agent_port;   
}
