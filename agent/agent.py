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

class SensorNetwork(py_environment.PyEnvironment):

    def __init__(self):
        self._action_spec = array_spec.BoundedArraySpec(
            shape=(), dtype=np.int32, minimum=0, maximum=1, name='send_message')
        self._observation_spec = array_spec.BoundedArraySpec(
            shape=(1,), dtype=np.int32, minimum=0, maximum=10, name='energy')
        self._state = 0
        self._episode_ended = False

    def action_spec(self):
        return self._action_spec

    def observation_spec(self):
        return self._observation_spec

    def _reset(self):
        self._state = 0
        self._episode_ended = False
        return ts.restart(np.array([self._state], dtype=np.int32))

    def _step(self, action):

        if self._episode_ended:
        # The last action ended the episode. Ignore the current action and start
        # a new episode.
            return self.reset()

        # Make sure episodes don't go on forever.
        if action == 1:
            self._episode_ended = True
        elif action == 0:
            new_card = np.random.randint(1, 11)
            self._state += new_card
        else:
            raise ValueError('`action` should be 0 or 1.')

        if self._episode_ended or self._state >= 21:
            reward = self._state - 21 if self._state <= 21 else -21
            return ts.termination(np.array([self._state], dtype=np.int32), reward)
        else:
            return ts.transition(
                np.array([self._state], dtype=np.int32), reward=0.0, discount=1.0)

class RandomAgent(fixed_policy_agent.FixedPolicyAgent):
  """An agent with a random policy and no learning."""

  def __init__(
      self,
      time_step_spec: ts.TimeStep,
      action_spec: types.NestedTensorSpec,
      debug_summaries: bool = False,
      summarize_grads_and_vars: bool = False,
      train_step_counter: Optional[tf.Variable] = None,
      num_outer_dims: int = 1,
      name: Optional[Text] = None,
  ):
    """Creates a random agent.

    Args:
      time_step_spec: A `TimeStep` spec of the expected time_steps.
      action_spec: A nest of BoundedTensorSpec representing the actions.
      debug_summaries: A bool to gather debug summaries.
      summarize_grads_and_vars: If true, gradient summaries will be written.
      train_step_counter: An optional counter to increment every time the train
        op is run.  Defaults to the global_step.
      num_outer_dims: same as base class.
      name: The name of this agent. All variables in this module will fall under
        that name. Defaults to the class name.
    """
    tf.Module.__init__(self, name=name)

    policy_class = random_tf_policy.RandomTFPolicy

    super(RandomAgent, self).__init__(
        time_step_spec,
        action_spec,
        policy_class=policy_class,
        debug_summaries=debug_summaries,
        summarize_grads_and_vars=summarize_grads_and_vars,
        train_step_counter=train_step_counter,
        num_outer_dims=num_outer_dims,
    )

class AgentFacade():
    
    class UnrewardedExperience():

        def __init__(self, state, action, message_id):
            self.state = state
            self.action = action
            self.message_id = message_id

    def __init__(self):
        self._env = SensorNetwork()
        self._train_env = tf_py_environment.TFPyEnvironment(self._env)
        self._agent = RandomAgent(self._train_env.time_step_spec(), self._train_env.action_spec())
        self._rnd_policy = random_py_policy.RandomPyPolicy(time_step_spec=None,
        action_spec=self._env.action_spec())
        self._unrw_buff = []
    

    def get_action(self, state, rewards):
        experiences = self._assemble_experience_list(rewards)
        self._update_policy(experiences)
        time_step = self._state_to_time_step(state)
        action = self._rnd_policy.action(time_step) 
        self._unrw_buff.append(self.UnrewardedExperience(state, action, 1))
        return ActionBean(action.action)

    def _state_to_time_step(self, state):
        return ts.restart(np.array([state.energy], dtype=np.int32))

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
            print("Esperienza ricevuta:\n\t Stato [energy:{0}] \n\t Azione [send_message:{1}] \n\t Reward: {2} ".format(e.observation.energy, e.step_type, e.reward))
            

class StateBean():

    def __init__(self, energy : int):
        # energy is a int value from 0 to 10
        self._energy = energy

    @property
    def energy(self):
        return self._energy

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
    print("Manda messagio: " + str(action.send_message))
    action = agent.get_action(state, rewards)
    print("Manda messagio: " + str(action.send_message))
    