cplusplus(h){{
#include <string>

#define SIMULATION_MSG_TOPIC "simulation_msg"
#define is_sim_msg(_msg) strcmp(_msg->getName(), SIMULATION_MSG_TOPIC) == 0

}}

enum SimulationMsgType{
    
    UNSPECIFIED = 0;

    // Message contains data to be forwarded to the sink
    DATA = 1;

    // Message contains neighbour state information
    ADVERTISEMENT = 2;

    // Message contains reward from sink
    REWARD = 3;

    }

message SimulationMsg extends cMessage{   
    int a=0;
}

cplusplus(SimulationMsg::SimulationMsg){{
    this->setName(SIMULATION_MSG_TOPIC); 
}}