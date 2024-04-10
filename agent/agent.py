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

from beans import ActionBean, RewardBean, RewardsBean, StateBean

class AgentFacade():
    
    class UnrewardedExperience():

        def __init__(self, state, action, message_id):
            self.state = state
            self.action = action
            self.message_id = message_id

    def __init__(self, max_neighbours=10):

        self._max_neighbours = max_neighbours
        node_spec = [
            # one list element for each tensor (state variable) 

            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32, name = "id"),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.int32, name = "energy_state"),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.bool, name = "power_state"),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.bool, name = "has_packet_in_buffer"),
            
        ]
        neighbour_spec = [
            tensor_spec.TensorSpec(shape=(max_neighbours), dtype=tf.float32, name = "id_neighbour"),
            tensor_spec.TensorSpec(shape=(max_neighbours), dtype=tf.int32, name = "energy_state_neighbour"),
            tensor_spec.TensorSpec(shape=(max_neighbours), dtype=tf.float32, name = "link_capacity")
        ]

        observation_spec = node_spec + neighbour_spec

        action_spec_root = tensor_spec.BoundedTensorSpec(shape=(1), dtype=tf.int32, minimum=0, maximum=1, name = "root_action")
        action_spec_change_power_state = tensor_spec.BoundedTensorSpec(shape=(1), dtype=tf.int32, minimum=0, maximum=1, name = "change_power_state")
        action_spec_send_message = tensor_spec.BoundedTensorSpec(shape=(max_neighbours), dtype=tf.float32 , minimum=0, maximum=1, name = "rates")

        time_step_spec = ts.time_step_spec(observation_spec)

        # TODO: make the agent implementation easily configurable
        agent_root = random_agent.RandomAgent(
            time_step_spec=time_step_spec,
            action_spec=action_spec_root)
        
        agent_change_power_state = random_agent.RandomAgent(
            time_step_spec=time_step_spec,
            action_spec=action_spec_change_power_state)
        
        agent_send_message = random_agent.RandomAgent(
            time_step_spec=time_step_spec,
            action_spec=action_spec_send_message)
        
        self._root = DecisionTreeConsultant(agent_root, "root")
        chang_power_state_node = DecisionTreeConsultant(agent_change_power_state, "change_power_state")
        send_message_node =DecisionTreeConsultant(agent_send_message, "send_message")
        self._root.add_choice(chang_power_state_node)
        self._root.add_choice(send_message_node)

        self._unrw_buff = []
    

    def get_action(self, state_bean, rewards_bean):
        return self._get_action(state_bean, rewards_bean)
    
    def _get_action(self, state, rewards):

        # updates agent policy using new rewards
        experiences = self._assemble_experience_list(rewards)
        #self._update_policy(experiences)
        self._root.train(experiences)

        # Computes TimeStep object by resetting the environment
        # at the given state
        # and uses it to get the action
        time_step = state.to_tensor(self._max_neighbours)
        action = []
        self._root.get_decisions(parent_state=time_step, decision_path=action)

        # Uses the action-state pair among the experiences waiting to be rewarded
        self._unrw_buff.append(self.UnrewardedExperience(state, action, 1))
        
        action_bean = ActionBean()
        action_bean.changePowerState = action

        return action_bean

    def _state_to_time_step(self, state):
        return ts.restart(
            observation=state.to_tensor(self._max_neighbours)
        )

    def _assemble_experience_list(self, rewards):
        experiences = []
        for r in rewards:
            for ue in self._unrw_buff:
                if r.message_id == ue.message_id:
                    experience = Experience(
                        state=ue.state.to_tensor(self._max_neighbours),
                        decision_path=ue.action,
                        message_id=ue.message_id,
                        reward=r.reward)
                    experiences.append(experience)
        return experiences

    def _assemble_experience(self, state, action, reward):
        return trajectory.Trajectory(
            step_type=ts.StepType.MID,
            observation=state,
            action=action,
            policy_info=(),
            next_step_type=ts.StepType.MID,
            reward=reward,
            discount=1.0)

    def _update_policy(self, experiences):
        for e in experiences:
            self._agent.train(e)
            print("Esperienza ricevuta:\n\t Stato [energy:{0}] \n\t Azione [send_message:{1}] \n\t Reward: {2} ".format(e.observation.energy, e.action.action[0][0], e.reward))
            
# Test main
if __name__ == '__main__':
    agent = AgentFacade()
    neighbour = StateBean(energy=1)
    state = StateBean(energy=1)
    state.add_neighbour(neighbour)
    rewards = [RewardBean(1, 10)]
    action = agent.get_action(state, [])
    print("Manda messagio: " + str(action.send_message))
    action = agent.get_action(state, rewards)
    print("Manda messagio: " + str(action.send_message))
    