#include "agent_client_pybind.h"
#include "agent_client.h"
#include "python_interpreter.h"
#include <omnetpp.h>
#include <string.h>

Define_Module(AgentClientPybind);

AgentClientPybind::AgentClientPybind()
{
    PythonInterpreter::getInstance()->use();

    // preloads the agent module to speed up simulation execution
    // (simulation startup will be slower)
    this->agent = py::module_::import("agent");
}

void AgentClientPybind::handleActionRequest(ActionRequest *msg)
{
    // TODO: connect this code with the real agent implementation #15
    
    py::none none;
    int ret;
    py::object get_action_ret;

    //get_action_ret = this->agent.attr("get_action")(none, none);
    //ret = get_action_ret.cast<int>();
    //EV << "get_action: " << ret << endl;

    EV << "Agent client received action request" << endl;
}

AgentClientPybind::~AgentClientPybind()
{
    this->agent.release();
    
    // unregisters from the python interpreter
    PythonInterpreter::getInstance()->put();
}
