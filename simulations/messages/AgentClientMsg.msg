cplusplus(h){{
#include <string>

#define AGENTC_MSG_TOPIC "agent_client_msg"
#define is_agentc_msg(_msg) strcmp(_msg->getName(), AGENTC_MSG_TOPIC) == 0

}}

enum AgentClientMsgKind{
    

   // /**
  //   * Request the agent what action the node should carry out.
  //   * The node must send the state and the rewards.
  //  */
    ACTION_REQUEST = 1;

  //  /**
   //  * Response to the node with the action the agent has chosen.
   // */
    ACTION_RESPONSE = 2;
    }

message AgentClientMsg extends cMessage{   
    int a=0;
}

cplusplus(AgentClientMsg::AgentClientMsg){{
    this->setName(AGENTC_MSG_TOPIC); 
}}