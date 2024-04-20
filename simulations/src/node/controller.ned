package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    parameters:
        int id;
        int ask_action_timeout_delta @unit(ms); // timeout delta for asking action (in sim time)
        double battery_capacity @unit(mAh); // battery capacity
        int data_buffer_capacity; // how many data msgs fit in the buffer
        int max_neighbours; // how many neighbours the node can keep track of at most
        double link_cap; // link capacity
    gates:
        inout network_port[10];
        inout src_port[1];
        inout sink_port[1];
        inout agent_port;   
}
