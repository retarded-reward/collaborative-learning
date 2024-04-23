package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    parameters:
        int number_of_ports @value(number-of-ports);//=default(1);
        int ask_action_timeout_delta @unit(ms); // timeout delta for asking action (in sim time)
        int data_buffer_capacity; // how many data msgs fit in the buffer
        int max_neighbours; // how many neighbours the node can keep track of at most
        double link_cap @unit(Mbps); // link capacity
        object power_models; // defines power consumptions for node operations
        object power_source_models;
    gates:
        inout network_port[number_of_ports];
        inout agent_port;   
}
