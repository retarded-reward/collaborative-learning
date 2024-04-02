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

from beans import ActionBean, RewardBean, StateBean

class AgentFacade():
    
    class UnrewardedExperience():

        def __init__(self, state, action, message_id):
            self.state = state
            self.action = action
            self.message_id = message_id

    def __init__(self):
        
        observation_spec = [

            # one list element for each tensor (state variable) 
            
            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.int32),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.bool),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32),
            tensor_spec.TensorSpec(shape=(1), dtype=tf.string),
            [
                [
                    tensor_spec.TensorSpec(shape=(1), dtype=tf.float32),
                    tensor_spec.TensorSpec(shape=(1), dtype=tf.int32),
                    tensor_spec.TensorSpec(shape=(1), dtype=tf.bool),
                    tensor_spec.TensorSpec(shape=(1), dtype=tf.float32),
                    tensor_spec.TensorSpec(shape=(1), dtype=tf.string),
                    # NOTE: it expects that the list of neighbours of each neighbour
                    # is empty. This is because I don't know how to recursively define
                    # the observation_spec for the neighbours of each neighbour.
                    []
                ]
            ]
        ]
        action_spec = [
            
            # TODO: 
            #       Predictions return an object with same shape as the action_spec.
            #       This means that if we give one tensor to each action, predictions will
            #       return an entire list of action. Same result is achieved if we use a
            #       single tensor with one dimension for each action.
            #
            #       This means that we are unable to choose one different type of action
            #       at each policy interrogation 
            #       (i.e we can't choose to predict send_message without predicting on/off state change).
            #
            #       One possible solution could leverage a hierarchical structure of actions,
            #       where the agents first decides which kind of action must be taken, and then
            #       chooses the specific value for that action.
            #       An example: agent chooses from {send_message, change_power_state}. If it 
            #       chooses send_message, it then predicts the tensor (which_neighbour, send_rate),
            #       else it chooses the power state from {on, off}.
            #       This solution brings a more difficult handling of the reward. How should the reward
            #       distribute among the hierachy levels? Should we use more agents, one for each level?

            # TODO: How to handle actions with variable action space? 
            #       (i.e: choose a relay node from a variable set of neighbours)

            # one list element for each tensor (action)

            tensor_spec.BoundedTensorSpec(
            shape=(1), dtype=tf.int32, minimum=0, maximum=1)
        ]
        time_step_spec = ts.time_step_spec(observation_spec)

        # TODO: make the agent implementation easily configurable
        self._agent = random_agent.RandomAgent(
            time_step_spec=time_step_spec,
            action_spec=action_spec)
        self._unrw_buff = []
    

    def get_action(self, state, rewards):

        # updates agent policy using new rewards
        experiences = self._assemble_experience_list(rewards)
        self._update_policy(experiences)

        # Computes TimeStep object by resetting the environment
        # at the given state
        # and uses it to get the action
        time_step = self._state_to_time_step(state)
        action = self._agent.policy.action(time_step)

        # Uses the action-state pair among the experiences waiting to be rewarded
        self._unrw_buff.append(self.UnrewardedExperience(state, action, 1))
        
        return ActionBean(action.action)

    def _state_to_time_step(self, state):
        return ts.restart(
            observation=state.to_tensor()
        )

    def _assemble_experience_list(self, rewards):
        experiences = []
        for r in rewards:
            for ue in self._unrw_buff:
                if r.message_id == ue.message_id:
                    experience = self._assemble_experience(ue.state, ue.action, r.reward)
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
            
if __name__ == '__main__':
    agent = AgentFacade()
    neighbour = StateBean(energy=1)
    state = StateBean(energy=1)
    state.add_neighbour(neighbour)
    rewards = [RewardBean(1, 10)]
    action = agent.get_action(state, [])
    print("Manda messagio: " + str(action.send_message[0][0]))
    action = agent.get_action(state, rewards)
    print("Manda messagio: " + str(action.send_message[0][0]))
    