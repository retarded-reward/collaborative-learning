#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include <omnetpp.h>
#include <string>
#include "ActionRequest_m.h"

using namespace omnetpp;
using namespace std;

enum class AgentClientMsgKind : short {
    UNSPECIFIED = 0,

    /**
     * Request the agent what action the node should carry out.
     * The node must send the state and the rewards.
    */
    ACTION_REQUEST,

    /**
     * Response to the node with the action the agent has chosen.
    */
    ACTION_RESPONSE,
};

/**
 * Interface of clients to the agent.
 * An agent client is a node component able to interact with the 
 * RL agent.
 * A node can drive an agent client by sending it requests and waiting
 * for responses.
*/
class AgentClient : public cSimpleModule {
    public:
        /**
         * Only messages with this topic as their name will be processed by the agent
         * client.
        */
        static const string MSG_TOPIC;

    protected:
        virtual void handleActionRequest(ActionRequest *msg) = 0;
        void initialize() override;
        void handleMessage(cMessage *msg) override;
};

#endif // AGENT_CLIENT_H