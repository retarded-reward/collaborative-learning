#include "agent_client.h"

const string AgentClient::MSG_TOPIC = "agent_client_msg";

void AgentClient::initialize()
{

}

void AgentClient::handleMessage(cMessage *msg)
{
    if (strcmp(msg->getName(), AgentClient::MSG_TOPIC.c_str())){
        
        EV_WARN << "Agent client received message with name "
         << msg->getName() 
         << " but it can process only messages with name "
         << AgentClient::MSG_TOPIC.c_str();
        goto agentc_handleMessage_exit;
    }

    switch (msg->getKind())
    {
        case (int) AgentClientMsgKind::ACTION_REQUEST:
            handleActionRequest((ActionRequest *) msg);
            break;
        
        default:
            EV_ERROR << "Agent client received a message with unrecognized kind "
            << msg->getKind();
            break;
    }
    
agentc_handleMessage_exit:
    delete msg;
}

