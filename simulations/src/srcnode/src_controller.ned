package org.cl.simulations.srcnode;

// SrcNode control logic such as generating packets and sending them to the network
simple SrcController
{
    parameters:
        volatile double send_interval;
    gates:
        inout network_port[1];
}
