package org.cl.simulations.node.queue;

// a priority queue with a bounded capacity.
// It can be connected to multiple servers.
simple Queue {
    parameters:
        int capacity;
        int priority = default(0);
        @display("i=block/queue,q=data_buffer");
    gates:
        inout servers_inout[];
        input data_in;
}