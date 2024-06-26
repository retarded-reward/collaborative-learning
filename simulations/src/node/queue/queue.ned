package org.cl.simulations.node.queue;

// a priority queue with a bounded capacity.
// It can be connected to multiple servers.
simple Queue {
    parameters:
        int capacity;
        int priority = default(0);
        @display("i=block/queue,q=data_buffer");
        
        @signal[queue*_pop_percentage](type=long);
        @statisticTemplate[queue_pop_percentage_over_time](record=vector,mean);
        
        @signal[queue*_queue_time](type=long);
        @statisticTemplate[queue_time_over_time](record=vector,mean);

        @signal[queue*_pkt_drop](type=long);
        @statisticTemplate[queue_pkt_drop](record=count);
        @signal[queue*_pkt_inbound](type=long);
        @statisticTemplate[queue_pkt_inbound](record=count);
        @signal[queue*_pkt_drop_percentage](type=long);
        @statisticTemplate[queue_pkt_drop_percentage_over_time](record=vector,mean);


    gates:
        inout servers_inout[];
        input data_in;
}