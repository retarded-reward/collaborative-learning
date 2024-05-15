//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.cl.simulations.node;

import ned.IdealChannel;
import org.cl.simulations.node.agentc.AgentClient;
import org.cl.simulations.node.Controller;
import org.cl.simulations.node.queue.Queue;

// a node of the network
module Node
{
    parameters:
        int number_of_ports  @value(number-of-ports);//=default(1);
        int num_queues @value(num_queues);
        
        // statistics
        @statistic[avg_cost_per_mWh](source=warmup(sum(energy_expense)/sum(energy_consumption)); record=mean; checkSignals=false; autoWarmupFilter=false);

        @statistic[cumulative_energy_expense](source=warmup(sum(energy_expense)); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        @statistic[cumulative_energy_potential_expense](source=warmup(sum(energy_potential_expense)); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        @statistic[cumulative_energy_consumption](source=warmup(sum(energy_consumption)); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        
        @statistic[ewma_energy_expense_per_mWh_over_time](source=ewma(warmup(sum(energy_expense)/(sum(energy_consumption)))); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        @statistic[ewma_energy_expense_per_second_over_time](source=ewma(warmup(sumPerSimtime(energy_expense))); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        @statistic[ewma_energy_consumption_per_second_over_time](source=ewma(warmup(sumPerSimtime(energy_consumption))); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        @statistic[battery_charge_level_over_time](source=battery_charge_level; record=vector; checkSignals=false);
        @statistic[ewma_energy_saving_over_time](source=ewma(100 - (warmup(sum(energy_expense)/(sum(energy_potential_expense))) * 100)); record=vector,last; checkSignals=false; autoWarmupFilter=false);
        
        @statistic[cumulative_reward_over_time](source=sum(reward); record=vector,last; checkSignals=false);
        @statistic[reward_over_time](source=reward; record=vector; checkSignals=false);
        
        //@statistic[service_interval](record=vector; checkSignals=false);
        //@statistic[pkt_arrival_time](record=vector; checkSignals=false);
        @statistic[ewma_utilization_over_time](source=ewma(avg(service_interval)/avg(sinceLast(pkt_arrival_time)) * 100); record=vector;last; checkSignals=false);
        // actions are recorded as follows:
        // 0: do nothing
        // i * num_power_sources + j + 1: select queue i and power source j 
        @statistic[action_over_time](source=action; record=vector,histogram; checkSignals=false);
        
        @statistic[do_nothing_count_over_time](source=count(do_nothing); record=vector,last; checkSignals=false);
        @statistic[send_battery_count_over_time](source=count(send_battery); record=vector,last; checkSignals=false);
        @statistic[send_powerchord_count_over_time](source=count(send_powerchord); record=vector,last; checkSignals=false);
        @statistic[send_count_over_time](source=count(send_battery) + count(send_powerchord); record=vector,last; checkSignals=false);
    gates:
        output network_out[number_of_ports] @loose;
        input queue_ports[num_queues] @loose;
    submodules:
        controller: Controller{
            num_queues = parent.num_queues;
        };
        agent: AgentClient{
            num_of_queues = parent.num_queues;
        };
        queues[num_queues]: Queue;
    connections allowunconnected:
        agent.port <--> IdealChannel <--> controller.agent_port; 
        for i=0..number_of_ports-1 {
            network_out[i] <-- controller.network_port[i];
        }
        for i=0..num_queues-1 {
            controller.queue_ports++ <--> queues[i].servers_inout++;
            queue_ports[i] --> queues[i].data_in;
        }
}
