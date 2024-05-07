#include "agent_client_pybind.h"
#include "agent_client.h"
#include "python_interpreter.h"
#include <omnetpp.h>
#include <cstddef>

Define_Module(AgentClientPybind);

AgentClientPybind::AgentClientPybind()
{
    PythonInterpreter::getInstance()->use();

    // preloads the agent module to speed up simulation execution
    // (simulation startup will be slower)
    this->agent_module = py::module_::import("agent");
    this->agent = this->agent_module.attr("AgentFacade")();
}

void AgentClientPybind::state_msg_to_bean(const NodeStateMsg &state, py::object bean){

    bean.attr("energy_level") = state.getEnergy_percentage();
    bean.attr("charge_rate") = state.getCharge_rate_percentage();
    for (int i = 0; i < state.getQueue_pop_percentageArraySize(); i ++){
        bean.attr("add_queue_state")(state.getQueue_pop_percentage(i));
    }

}

void AgentClientPybind::reward_msg_to_bean(const RewardMsg &msg, py::object reward_bean)
{
    reward_bean.attr("reward") = msg.getValue();
}

void AgentClientPybind::action_bean_to_msg(py::object bean, ActionResponse *msg){

    msg->setSend_message(bean.attr("send_message").cast<bool>());
    msg->setSelect_power_source((SelectPowerSource)(bean.attr("power_source").cast<int>() - 1));
    msg->setQueue(bean.attr("queue").cast<int>());
}

void AgentClientPybind::handleActionRequest(ActionRequest *msg)
{    
    py::object state_bean = this->agent_module.attr("StateBean")();
    py::object reward_bean = this->agent_module.attr("RewardBean")();
    py::object action_bean;
    py::object current_action_bean;
    ActionResponse *response;

    EV_DEBUG << "Agent client received action request" << endl;

    // converts request params in state and reward beans
    state_msg_to_bean(msg->getState(), state_bean);
    reward_msg_to_bean(msg->getReward(), reward_bean);
    // interrogates agent for the next action
    action_bean = this->agent.attr("get_action")(state_bean, reward_bean);

    // prints output of the agent to console
    EV_DEBUG << "Agent output:" << endl;
    EV_DEBUG << PythonInterpreter::getInstance()->pyStdStreamsRedirect->outString();
    EV_DEBUG << "end of agent output" << endl;
    // converts the action bean in a ActionResponse message and send it
    // back to the controller
    response = new ActionResponse();
    action_bean_to_msg(action_bean, response);
    this->send(response, "port$o");
}

AgentClientPybind::~AgentClientPybind()
{
    this->agent.release();
    this->agent_module.release();
    
    // unregisters from the python interpreter
    PythonInterpreter::getInstance()->put();
}
