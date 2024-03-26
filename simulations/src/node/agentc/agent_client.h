#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include <omnetpp.h>
#include <string>
#include "ActionRequest_m.h"

using namespace omnetpp;
using namespace std;

enum class AgentClientMsgKind : short {
    UNSPECIFIED = 0,
    ACTION_REQUEST,
    ACTION_RESPONSE,
};

class AgentClient : public cSimpleModule {
    public:
        /**
         * Only messages with this topic as their name will be processed by the agent
         * client.
        */
        static const string MSG_TOPIC;
};

#endif // AGENT_CLIENT_H