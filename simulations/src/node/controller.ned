package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    gates:
        inout network_port[12]; //Port 0 reserved for srcNode and port 1 for sink
        inout agent_port;   
}
