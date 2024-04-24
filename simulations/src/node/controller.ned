package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    parameters:
        int number_of_ports @value(number-of-ports);//=default(1);
        double pkt_drop_cost; // packet drop cost of each queue
        double queue_occ_cost; // queue occupancy cost of each queue
        double energy_cost; // energy cost of each power source
        double pkt_drop_penalty_weight; // weight of packet drop in the reward function
        double queue_occ_penalty_weight; // weight of queue occupancy in the reward function
        double energy_penalty_weight; // weight of energy cost in the reward function
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
