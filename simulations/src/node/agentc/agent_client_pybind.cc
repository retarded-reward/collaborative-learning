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
    this->agent_module = py::module_::import("agent");
    this->agent = this->agent_module.attr("AgentFacade")();
}

void AgentClientPybind::state_msg_to_bean(NodeStateMsg state, py::object bean){
    
    py::object neighbour_state_bean;
    NodeStateMsg neighbour_state;

    bean.attr("energy") = state.getEnergy();
    bean.attr("has_packet_in_buffer") = state.getHas_packet_in_buffer();
    bean.attr("power_state") = (int) state.getPower_state();
    for (int i = 0; i < state.getNeighboursArraySize(); i++)
    {
        neighbour_state = state.getNeighbours(i);
        neighbour_state_bean = this->agent_module.attr("StateBean")();
        state_msg_to_bean(neighbour_state, neighbour_state_bean);
        bean.attr("add_neighbour")(neighbour_state_bean);
    }
}

void AgentClientPybind::handleActionRequest(ActionRequest *msg)
{    
    py::object state_bean = this->agent_module.attr("StateBean")();
    py::object rewards_bean = this->agent_module.attr("RewardsBean")();
    py::object action_bean;

    EV_DEBUG << "Agent client received action request" << endl;

    // converts request params in state and reward beans
    state_msg_to_bean(msg->getState(), state_bean);
    for (int i = 0; i < msg->getRewardArraySize(); i++)
    {
        const SinkRewardMsg &current_reward = msg->getReward(i);
        rewards_bean.attr("add_reward")(current_reward.getMessage_id(),
         current_reward.getValue());
    }

    // interrogates agent for the next action
    action_bean = this->agent.attr("get_action")(state_bean, rewards_bean);
    EV_DEBUG << "Agent chose action" << action_bean.attr("send_message").cast<int>() << endl;

    // TODO: convert the action bean in a ActionResponse message and send it
    // back to the controller

}

AgentClientPybind::~AgentClientPybind()
{
    this->agent.release();
    this->agent_module.release();
    
    // unregisters from the python interpreter
    PythonInterpreter::getInstance()->put();
}
