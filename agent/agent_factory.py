from enum import Enum
from tf_agents.agents.random import fixed_policy_agent, random_agent
from typing import Callable, Union

class AgentEnum(Enum):
    RANDOM_AGENT = 1

class AgentFactory():
    
    @staticmethod
    def create_agent(agent_description, time_step_spec, action_spec):
        agent_type = agent_description["agent_type"]
        match agent_type:
            case AgentEnum.RANDOM_AGENT:
                return random_agent.RandomAgent(time_step_spec, action_spec)
            case _:
                raise Exception("Agent type not supported")
