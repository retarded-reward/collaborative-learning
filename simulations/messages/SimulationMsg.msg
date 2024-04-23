cplusplus(h){{
#include <string>

#define SIMULATION_MSG_TOPIC "simulation_msg"
#define is_sim_msg(_msg) strcmp(_msg->getName(), SIMULATION_MSG_TOPIC) == 0

}}

enum SimulationMsgKind{
    
    // Message contains data to be forwarded to the sink
    DATA_MSG = 1;

    // Message contains reward 
    REWARD_MSG = 2;

}

message SimulationMsg extends cMessage{   
    int a=0;
}

cplusplus(SimulationMsg::SimulationMsg){{
    this->setName(SIMULATION_MSG_TOPIC); 
}}