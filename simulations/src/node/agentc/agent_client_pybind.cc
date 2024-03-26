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

void AgentClientPybind::initialize()
{

}

void AgentClientPybind::handleActionRequest(ActionRequest *msg)
{
    // TODO: connect this code with the real agent implementation #15
    
    py::none none;
    int ret;
    py::object get_action_ret;

    get_action_ret = this->agent.attr("get_action")(none, none);
    ret = get_action_ret.cast<int>();
    EV << "get_action: " << ret << endl;
}

void AgentClientPybind::handleMessage(cMessage *msg)
{
    if (strcmp(msg->getName(), AgentClient::MSG_TOPIC.c_str())){
        
        EV_WARN << "Agent client received message with name "
         << msg->getName() 
         << " but it can process only messages with name "
         << AgentClient::MSG_TOPIC.c_str();
        goto agentc_handleMessage_exit;
    }

    switch (msg->getKind())
    {
        case (int) AgentClientMsgKind::ACTION_REQUEST:
            this->handleActionRequest((ActionRequest *) msg);
            break;
        
        default:
            EV_ERROR << "Agent client received a message with unrecognized kind "
            << msg->getKind();
            break;
    }
    
agentc_handleMessage_exit:
    delete msg;
}

AgentClientPybind::~AgentClientPybind()
{
    this->agent.release();
    
    // unregisters from the python interpreter
    PythonInterpreter::getInstance()->put();
}
