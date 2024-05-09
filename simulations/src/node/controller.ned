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
        int data_buffer_capacity; // how many data msgs fit in the buffer
        int max_neighbours; // how many neighbours the node can keep track of at most
        double link_cap @unit(Mbps); // link capacity
        object power_models; // defines power consumptions for node operations
        object power_source_models;
        volatile double battery_charge_rate_distribution;
        double charge_battery_timeout_delta @unit(s);

        // statistics
        @statistic[avg_cost_per_mWh](source=sum(energy_expense)/sum(energy_consumption); record=mean; checkSignals=false);

        @statistic[ewma_energy_expense_per_mWh_over_time](source=ewma(sum(energy_expense)/(sum(energy_consumption))); record=vector; checkSignals=false);
        @statistic[ewma_energy_expense_per_second_over_time](source=ewma(sumPerDuration(energy_expense)); record=vector; checkSignals=false);
        @statistic[energy_consumption_per_second_over_time](source=sumPerDuration(energy_consumption); record=vector; checkSignals=false);
        //@statistic[avg_utilization_over_time](source=timeavg(avg_arrival_rate/avg(service_interval)) * 100; record=vector; checkSignals=false);
        @statistic[battery_charge_level_over_time](source=battery_charge_level; record=vector; checkSignals=false);
        @statistic[cumulative_reward_over_time](source=sum(reward); record=vector; checkSignals=false);
        @statistic[reward_over_time](source=reward; record=vector; checkSignals=false);

    gates:
        output network_port[number_of_ports];
        inout agent_port;
        inout queue_ports[];   
}
