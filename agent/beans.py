from enum import IntEnum

import tensorflow as tf
from typing import Iterable
from tf_agents.specs.tensor_spec import TensorSpec, BoundedTensorSpec
import warnings
import numpy as np

class StateBean():

    def __init__(self,
        energy_level : float = 0, 
        queue_state : Iterable[float] = [],
        charge_rate : float = 0,
        ):
        
        self._energy_level = energy_level
        """
        Percentage battery energy level available for the node to perform actions.
        energy_level is a float value from 0 to 1.
        """

        self._queue_state = queue_state
        """
        Percentage of the buffer capacity that is currently used.
        queue_state is a float value from 0 to 1.
        """

        self._charge_rate = charge_rate
        """
        Rate at which the battery is being charged.
        charge_rate is a float value from 0 to 1.
        """
    
    @property
    def energy_level(self):
        return self._energy_level
    
    @energy_level.setter
    def energy_level(self, energy_level):
        self._energy_level = energy_level

    @property
    def queue_state(self):
        return self._queue_state
    
    @queue_state.setter
    def queue_state(self, queue_state):
        self._queue_state = queue_state

    @property
    def charge_rate(self):
        return self._charge_rate
    
    @charge_rate.setter
    def charge_rate(self, charge_rate):
        self._charge_rate = charge_rate

    def add_queue_state(self, queue_state):
        self._queue_state.append(queue_state)

    def clean_queue(self):
        self._queue_state = []

    @staticmethod
    def observation_spec(n_queues: int) -> TensorSpec:
        """
        State is described as a single vector of ints.
        The vector has the following structure:
        [energy_level, queue_state_1, ..., queue_state_n, charge_rate]
        All values are integer percentages from 0 to 100.
        """
        return BoundedTensorSpec(shape=(1 + n_queues + 1), 
                                 dtype=np.int32, 
                                 minimum=(0, *[0 for i in range(n_queues)], 0), 
                                 maximum=(100, *[100 for i in range(n_queues)], 100), 
                                 name = "state_spec")
        
        

    def to_tensor(self, n_queues : int):
        """
        See observation_spec for the structure of the tensor.
        """
        
        # NOTE: keep in sync with observation_spec
        #      and with the state variables in the constructor

        if len(self.queue_state) != n_queues:
            warnings.warn("The number of queues in the state does not match the number of queues in the environment. The state will be padded with zeros or truncated to match the number of queues in the environment")
        
        #queue_state = (self._queue_state[:n_queues] + (max(n_queues - len(self._queue_state), 0)) * [0])
        node_spec = tf.constant(shape=(1 + n_queues + 1), 
                                dtype=tf.int32, 
                                name = "state", 
                                value=[int(self.energy_level), *[int(qstate) for qstate in self._queue_state], int(self.charge_rate)])
        return node_spec
    
    def __str__(self):
        return "StateBean(energy_level={}, queue_state={}, charge_rate={})".format(
            self.energy_level, self.queue_state, self.charge_rate)
    
class RewardBean():

    def __init__(self, reward : int = None):
        # id of the message corresponding to the reward
        self._reward = reward
    
    @property
    def reward(self):
        return self._reward
    
    @reward.setter
    def reward(self, reward):
        self._reward = reward

    def __str__(self):
        return "RewardBean(reward={})".format(self.reward)
    

class ActionBean():

    class SendEnum(IntEnum):
        DO_NOTHING = 0
        SEND_MESSAGE = 1
    

    class PowerSourceEnum(IntEnum):
        NO_SOURCE = -1
        BATTERY = 0
        POWER_CHORD = 1

    def __init__(self, send_message : SendEnum, power_source : PowerSourceEnum = PowerSourceEnum.NO_SOURCE, queue : int = -1):
        self._send_message = send_message
        self._power_source = power_source
        self._queue = queue

    @property
    def send_message(self):
        return self._send_message
    
    @send_message.setter
    def send_message(self, send_message):
        self._send_message = send_message

    @property
    def power_source(self):
        return self._power_source
    
    @power_source.setter
    def power_source(self, power_source):
        self._power_source = power_source

    @property
    def queue(self):
        return self._queue
    
    @queue.setter
    def queue(self, queue):
        self._queue = queue
    
    def __str__(self):
        return "ActionBean(send_message={}, power_source={}, queue={})".format(
            self.send_message, self.power_source, self.queue)
    