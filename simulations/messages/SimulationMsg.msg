cplusplus(h){{
#include <string>

#define SIMULATION_MSG_TOPIC "simulation_msg"
#define is_sim_msg(_msg) strcmp(_msg->getName(), SIMULATION_MSG_TOPIC) == 0

}}

enum SimulationMsgKind{
    
    // Message contains data to be forwarded to the sink
    DATA_MSG = 1;

    // Message contains neighbour state information
    ADVERTISEMENT_MSG = 2;

    // Message contains reward from sink
    REWARD_MSG = 3;

}

message SimulationMsg extends cMessage{   
    int a=0;
}

cplusplus(SimulationMsg::SimulationMsg){{
    this->setName(SIMULATION_MSG_TOPIC); 
}}