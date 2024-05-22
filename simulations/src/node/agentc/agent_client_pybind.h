#ifndef AGENT_CLIENT_PYBIND_H
#define AGENT_CLIENT_PYBIND_H

#include "agent_client.h"
#include <pybind11/embed.h>
#include "cpp_visibility_tools.h"
#include "ActionResponse_m.h"
#include <cstddef>

namespace py = pybind11;

class DLL_LOCAL AgentClientPybind : public AgentClient {
    protected:
        py::object agent;

        size_t num_of_queues;
        
        void state_msg_to_bean(const NodeStateMsg &msg, py::object bean);
        void reward_msg_to_bean(const RewardMsg &reward, py::object bean);
        void action_bean_to_msg(py::object bean, ActionResponse *msg);  
        
        void handleActionRequest(ActionRequest *msg) override;
        void initialize() override;
        
        void init_module_params();
        void init_python_interface();
    public:
        AgentClientPybind();
        ~AgentClientPybind();
};
    

#endif // AGENT_CLIENT_PYBIND_H

