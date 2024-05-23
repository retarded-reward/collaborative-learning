package org.cl.simulations.node;

// Node control logic, such as neighbour discovery, action attuation, etc.
simple Controller
{
    parameters:
        @display("i=block/app");
        int number_of_ports @value(number-of-ports);//=default(1);
        int num_queues;

        // Dictionary of reward term models.
        // A reward term model can be used to construct one or more reward terms.
        // Reward for the agent can be computed by combining all the reward terms.
        // A reward term model is an object made at least of the following members:
        // - weight: weight of the term in the reward computation;
        // - signal: a function that measures a quantity and returns the corresponding
        //           reward value for that term.
        // If the signal function needs parameters, they can be specified in the
        // reward term model as additional members.
        object reward_term_models;
        
        double ask_action_timeout_delta @unit(s); // timeout delta for asking action (in sim time)
        int max_neighbours; // how many neighbours the node can keep track of at most
        double link_cap @unit(Mbps); // link capacity
        object power_models; // defines power consumptions for node operations
        object power_source_models;
        volatile double battery_charge_rate_distribution;
        double charge_battery_timeout_delta @unit(s);
        
        // the illegal action penalty is the max measured reward times
        // this factor.
        double illegal_action_penalty_factor;

    gates:
        output network_port[number_of_ports];
        inout agent_port;
        inout queue_ports[];   
}
