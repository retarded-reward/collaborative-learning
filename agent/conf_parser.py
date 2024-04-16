import re
from agent_factory import AgentEnum

class ConfParser:

    pattern_random_agent = re.compile("random_agent|randomagent|random|random-agent")

    @staticmethod
    def parse_agent_from_json(json):
        ret = {}
        agent_type_raw = json["agent_type"].lower()
        if ConfParser.pattern_random_agent.match(agent_type_raw):
             ret["agent_type"] = AgentEnum.RANDOM_AGENT
        return ret