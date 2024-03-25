#include "agent_client_pybind.h"
#include "agent_client.h"
#include "python_interpreter.h"
#include <omnetpp.h>

Define_Module(AgentClientPybind);

AgentClientPybind::AgentClientPybind()
{
    PythonInterpreter::getInstance()->use();

    //preloads the agent module to speed up simulation execution
    this->agent = py::module_::import("agent");
}

void AgentClientPybind::initialize()
{

}

void AgentClientPybind::handleMessage(cMessage *msg)
{
    // TODO: Filter messagges
    // TODO: write action request handling in separate method
    py::none none;
    int ret;
    py::object get_action_ret;

    get_action_ret = this->agent.attr("get_action")(none, none);
    ret = get_action_ret.cast<int>();
    EV << "get_action: " << ret << endl;
    delete msg;
}

AgentClientPybind::~AgentClientPybind()
{
    this->agent.release();
    
    // unregisters from the python interpreter
    PythonInterpreter::getInstance()->put();
}
