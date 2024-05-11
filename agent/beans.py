from enum import IntEnum

import tensorflow as tf
from typing import Iterable
from tf_agents.specs.tensor_spec import TensorSpec
import warnings

class NodePowerState(IntEnum):
    OFF = 0
    ON = 1

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

    
    """
    Returns a list of tensors representing the state of the agent.
    Each tensor is a state variable.
    """
    def to_tensor(self, n_queues : int):
        
        # NOTE: keep in sync with observation_spec in AgentFacade constructor
        #      and with the state variables in the constructor

        if len(self.queue_state) != n_queues:
            warnings.warn("The number of queues in the state does not match the number of queues in the environment. The state will be padded with zeros or truncated to match the number of queues in the environment")
        
        queue_state = (self._queue_state[:n_queues] + (max(n_queues - len(self._queue_state), 0)) * [0])
        node_spec = (
            tf.constant(shape=(), dtype=tf.float32, name = "energy_level", value=self.energy_level),
            tf.constant(shape=(n_queues), dtype=tf.float32, name = "queue_state", value=queue_state),
            tf.constant(shape=(), dtype=tf.float32, name = "charge_rate", value=self.charge_rate)
        )

        #node_spec = tf.constant(shape=(1, 3), dtype=tf.float32, name = "state", value=[self.energy_level, self.queue_state[0], self.charge_rate])
            
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
    

# TODO: implement the other actions according to the model
class ActionBean():

    class SendEnum(IntEnum):
        DO_NOTHING = 0
        SEND_MESSAGE = 1
        

    class PowerSourceEnum(IntEnum):
        NO_SOURCE = 0
        BATTERY = 1
        POWER_CHORD = 2

    def __init__(self, send_message : SendEnum, power_source : PowerSourceEnum = PowerSourceEnum.NO_SOURCE, queue : int = 0):
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
    