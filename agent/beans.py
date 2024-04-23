# TODO: maybe is better to move creation of *_spec objects in static methods
#       of the beans. For example, the StateBean class knows what is the structure
#       of the state tensor, so it can provide a method that returns the observation_spec.

from enum import IntEnum

import tensorflow as tf
from typing import Iterable

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

    
    """
    Returns a list of tensors representing the state of the agent.
    Each tensor is a state variable.
    """
    def to_tensor(self, max_neighbours):
        
        # NOTE: keep in sync with observation_spec in AgentFacade constructor
        #      and with the state variables in the constructor

        node_spec = [
            tf.constant(shape=(1), dtype=tf.float32, name = "energy_level", value=self.energy_level),
            tf.constant(shape=(1), dtype=tf.float32, name = "queue_state", value=self.queue_state),
            tf.constant(shape=(1), dtype=tf.float32, name = "charge_rate", value=self.charge_rate)
        ]
            
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
        SEND_MESSAGE = 0
        DO_NOTHING = 1

    class PowerSourceEnum(IntEnum):
        BATTERY = 0
        POWER_CHORD = 1

    def __init__(self, send_message : SendEnum, power_source : PowerSourceEnum = None, queue : int = 0):
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
    

