#include "agent_client.h"
#include "AgentClientMsg_m.h"

void AgentClient::initialize()
{

}

void AgentClient::handleMessage(cMessage *msg)
{
    if (!is_agentc_msg(msg)){
        
        EV_WARN << "Agent client received message with name "
         << msg->getName() 
         << " but it can process only messages with name "
         << AGENTC_MSG_TOPIC << endl;
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

