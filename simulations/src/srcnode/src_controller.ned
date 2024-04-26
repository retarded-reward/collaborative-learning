package org.cl.simulations.srcnode;

// SrcNode control logic such as generating packets and sending them to the network
simple SrcController
{
    parameters:
        volatile double send_interval;
    gates:
        output network_port[1];
}
