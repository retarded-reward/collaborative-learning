import logging
from typing import Optional, Text
import tensorflow as tf
import numpy as np
import threading

from tf_agents.environments import py_environment
from tf_agents.specs import array_spec
from tf_agents.trajectories import time_step as ts
from tf_agents.environments import tf_py_environment
from tf_agents.specs import tensor_spec
from tf_agents.networks import sequential
from tf_agents.agents.dqn import dqn_agent
from tf_agents.agents.random import random_agent
from tf_agents.utils import common
from tf_agents.trajectories import trajectory
from tf_agents.policies import random_tf_policy
from tf_agents.policies import random_py_policy
from tf_agents.agents.random import fixed_policy_agent
from tf_agents.typing import types
from tf_agents.agents.tf_agent import TFAgent
from tf_agents.trajectories.time_step import TimeStep
from tf_agents.trajectories.time_step import StepType
from tf_agents.agents.tf_agent import LossInfo

class RandomAgent(TFAgent):
    """An agent with a random policy and no learning."""

    def __init__(
        self,
        time_step_spec: ts.TimeStep,
        action_spec: types.NestedTensorSpec,
        name: Optional[Text] = None,
    ):
        policy = random_tf_policy.RandomTFPolicy(time_step_spec, action_spec)
        super(RandomAgent, self).__init__(
            time_step_spec,
            action_spec,
            policy = policy,
            collect_policy = policy,
            train_sequence_length = None
        )

    def _train(self, experience, weights=None):
        print("Training...")

        # NOTE: counter must be incremented manually by exact one.
        #       See comments of TFAgent.train method for more details. 
        self.train_step_counter.assign_add(1)

        # NOTE: Loss should be computed by invoking the loss function of the policy
        return LossInfo(loss=tf.constant(0.0), extra=())

class AgentFacade():
    
    class UnrewardedExperience():

        def __init__(self, state, action, message_id):
            self.state = state
            self.action = action
            self.message_id = message_id

    def __init__(self):
        
        observation_spec = [

            # one list element for each tensor (state variable) 
            # TODO: maybe each state variable should be a tensor dimension instead?
            tensor_spec.TensorSpec(shape=(1), dtype=tf.float32)
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
        self._agent = RandomAgent(time_step_spec, action_spec)
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
            
# TODO: implement the other state variables according to the model
class StateBean():

    def __init__(self, energy : int):
        # energy is a int value from 0 to 10
        self._energy = energy

    @property
    def energy(self):
        return self._energy
    
    """
    Returns a list of tensors representing the state of the agent.
    Each tensor is a state variable.
    """
    def to_tensor(self):
        
        # keep in sync with observation_spec in AgentFacade constructor
        return [
            tf.constant(
                name="node_energy",
                value=self.energy,
                shape=(1),
                dtype=tf.float32
            )
        ]

class RewardBean():

    def __init__(self, message_id : int, reward : int):
        # id of the message corresponding to the reward
        self._message_id = message_id
        self._reward = reward

    @property
    def message_id(self):
        return self._message_id
    
    @property
    def reward(self):
        return self._reward
    
# TODO: implement the other actions according to the model
class ActionBean():
    
    def __init__(self, send_message : int):
        # send_message is a int value 0 or 1
        self._send_message = send_message

    @property
    def send_message(self):
        return self._send_message


if __name__ == '__main__':
    agent = AgentFacade()
    state = StateBean(1)
    rewards = [RewardBean(1, 10)]
    action = agent.get_action(state, [])
    print("Manda messagio: " + str(action.send_message[0][0]))
    action = agent.get_action(state, rewards)
    print("Manda messagio: " + str(action.send_message[0][0]))
    