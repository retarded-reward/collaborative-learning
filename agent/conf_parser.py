import re
from agent_factory import AgentEnum

class ConfParser:

    pattern_random_agent = re.compile("random_agent|randomagent|random|random-agent")
    pattern_dqn_agent = re.compile("dqn_agent|dqnagent|dqn|dqn-agent")

    @staticmethod
    def parse_agent_from_json(json):
        ret = {}
        agent_type_raw = json["agent_type"].lower()
        if ConfParser.pattern_random_agent.match(agent_type_raw):
            ret["agent_type"] = AgentEnum.RANDOM_AGENT
        elif ConfParser.pattern_dqn_agent.match(agent_type_raw):
            ret["agent_type"] = AgentEnum.DQN_AGENT
        return ret