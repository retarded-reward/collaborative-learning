from typing import Optional, Text

import numpy as np
import tensorflow as tf
from tf_agents.agents.random import fixed_policy_agent, random_agent
from tf_agents.agents.tf_agent import LossInfo, TFAgent
from tf_agents.policies import random_tf_policy
from tf_agents.specs import array_spec, tensor_spec
from tf_agents.trajectories import time_step as ts
from tf_agents.trajectories import trajectory
from tf_agents.trajectories.time_step import StepType, TimeStep
from tf_agents.typing import types
from tf_agents.utils import common
from decisions import DecisionTreeConsultant
from decisions import Experience
from typing import Callable, Union

from beans import ActionBean, RewardBean, StateBean

from agent_factory import AgentEnum, AgentFactory
from conf_parser import ConfParser
import json
class AgentFacade():
    
    
    class UnrewardedExperience():

        def __init__(self, state, action, message_id):
            self.state = state
            self.action = action
            self.message_id = message_id

    def __init__(self, max_neighbours = 10, agent_description = {"agent_type": AgentEnum.RANDOM_AGENT}):
        self._last_experience = None
        self._max_neighbours = max_neighbours
        node_spec = [
            # one list element for each tensor (state variable) 
            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32, name = "energy_level"),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32, name = "queue_state"),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32, name = "charge_rate")
        ]

        observation_spec = node_spec

        action_spec_root = tensor_spec.BoundedTensorSpec(shape=(), dtype=tf.int32, minimum=0, maximum=2, name = "choose_action")

        time_step_spec = ts.time_step_spec(observation_spec)

        # TODO: make the agent implementation easily configurable
        agent_root = AgentFactory.create_agent(
            agent_description = agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_root)
        
        '''
        agent_change_power_state = AgentFactory.create_agent(
            agent_description = agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_change_power_state)
        
        agent_send_message = AgentFactory.create_agent(
            agent_description = agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_send_message)
        '''
        
        self._root = DecisionTreeConsultant(agent_root, "root")
        #chang_power_state_node = DecisionTreeConsultant(agent_change_power_state, "change_power_state")
        #send_message_node =DecisionTreeConsultant(agent_send_message, "send_message")
        #self._root.add_choice(chang_power_state_node)
        #self._root.add_choice(send_message_node)
    

    def get_action(self, state_bean, rewards_bean):
        return self._get_action(state_bean, rewards_bean.rewards)
    
    def _get_action(self, state, reward):

        # updates agent policy using reward from previous action
        if(reward is not None):
            exp = Experience(self._last_experience[0], self._last_experience[1], reward)
            self._root.train([exp])

        # Computes TimeStep object by resetting the environment
        # at the given state
        # and uses it to get the action
        time_step = state.to_tensor(self._max_neighbours)
        action = []
        self._root.get_decisions(parent_state=time_step, decision_path=action)
        
        # Stores the last experience
        self._last_experience = (state, action)

        action_bean = self._decision_path_to_action_bean(action)

        return action_bean
    
    def _decision_path_to_action_bean(self, decision_path):
        action = int(decision_path[0].value.action)
        match action:
            case 0:
                return ActionBean(send_message=ActionBean.SendEnum.DO_NOTHING)
            case 1:
                return ActionBean(
                    send_message=ActionBean.SendEnum.SEND_MESSAGE, 
                    power_source=ActionBean.PowerSourceEnum.BATTERY)
            case 2:
                return ActionBean(
                    send_message=ActionBean.SendEnum.SEND_MESSAGE, 
                    power_source=ActionBean.PowerSourceEnum.POWER_CHORD)
            
                  
            
# Test main
if __name__ == '__main__':
    agent_desc = ConfParser.parse_agent_from_json(json.load(open("agent/agent_conf.json")))
    agent = AgentFacade(
        agent_description=agent_desc)
    state = StateBean(energy_level=1, queue_state=[1], charge_rate=0)
    action = agent.get_action(state, None)
    print("Azione scelta: " + str(action))
    reward = RewardBean(10)
    action = agent.get_action(state, reward)
    print("Azione scelta: " + str(action))
    