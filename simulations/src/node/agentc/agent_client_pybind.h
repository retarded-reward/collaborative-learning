#ifndef AGENT_CLIENT_PYBIND_H
#define AGENT_CLIENT_PYBIND_H

#include "agent_client.h"
#include <pybind11/embed.h>

namespace py = pybind11;

class AgentClientPybind : public AgentClient {
    private:
        py::module agent_module;
        py::object agent;
        void state_msg_to_bean(NodeStateMsg msg, py::object bean);
    protected:
        void handleActionRequest(ActionRequest *msg) override;
    public:
        AgentClientPybind();
        ~AgentClientPybind();
};
    

#endif // AGENT_CLIENT_PYBIND_H

