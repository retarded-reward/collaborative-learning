#ifndef AGENT_CLIENT_H
#define AGENT_CLIENT_H

#include <omnetpp.h>
#include "ActionRequest_m.h"

using namespace omnetpp;
using namespace std;

/**
 * Interface of clients to the agent.
 * An agent client is a node component able to interact with the 
 * RL agent.
 * A node can drive an agent client by sending it requests and waiting
 * for responses.
*/
class AgentClient : public cSimpleModule {
    public:
    protected:

        char *implementation;

        void init_module_params();
    
        virtual void handleActionRequest(ActionRequest *msg) = 0;
        void initialize() override;
        void handleMessage(cMessage *msg) override;
    
};

#endif // AGENT_CLIENT_H