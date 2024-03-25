#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include <omnetpp.h>
#include <string>

using namespace omnetpp;
using namespace std;

class AgentClientMsg{
    public:
        static const AgentClientMsg ACTION_REQUEST;
        static const AgentClientMsg ACTION_RESPONSE;
    private:
        string topic;   // Agent will process messagges with this topic as their name

        AgentClientMsg(string topic);

    public:
        string getTopic() const;
};

class AgentClient : public cSimpleModule {

};

#endif // AGENT_CLIENT_H