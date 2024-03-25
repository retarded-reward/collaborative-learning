#include "agent_client.h"

string AgentClientMsg::getTopic() const {
    return this->topic;
}

AgentClientMsg::AgentClientMsg(string topic) {
    this->topic = topic;
}

const AgentClientMsg AgentClientMsg::ACTION_REQUEST = AgentClientMsg("action request");
const AgentClientMsg AgentClientMsg::ACTION_RESPONSE = AgentClientMsg("action response");

