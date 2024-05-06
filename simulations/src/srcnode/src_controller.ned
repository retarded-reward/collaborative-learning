package org.cl.simulations.srcnode;

// SrcNode control logic such as generating packets and sending them to the network
simple SrcController
{
    parameters:
        volatile double send_interval;
        int min_pkt_size;
        int max_pkt_size;
    gates:
        output network_port[1];
}
