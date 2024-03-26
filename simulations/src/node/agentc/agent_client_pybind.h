#ifndef AGENT_CLIENT_PYBIND_H
#define AGENT_CLIENT_PYBIND_H

#include "agent_client.h"
#include <pybind11/embed.h>

namespace py = pybind11;

class AgentClientPybind : public AgentClient {
    protected:
        void initialize() override;
        void handleMessage(cMessage *msg) override;
    private:
        py::module agent;
        
        void handleActionRequest(ActionRequest *msg);
    public:
        AgentClientPybind();
        ~AgentClientPybind();
};
    

#endif // AGENT_CLIENT_PYBIND_H

