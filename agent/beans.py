from enum import IntEnum

import tensorflow as tf

class NodePowerState(IntEnum):
    OFF = 0
    ON = 1

class StateBean():

    def __init__(self,
     energy : float = 0, power_state : NodePowerState = 0,
      has_packet_in_buffer : bool = False,
     link_capacity : float = -1, id : str = ""):
        
        self._energy = energy
        """
        Energy available for the node to perform actions.
        energy is a int value from 0 to 10.
        """

        self._power_state = power_state
        """
        Power state of the node. Can be 0 (OFF) or 1 (ON).
        If node is ON, it can exchange messages to neighbours.
        If node is OFF, it can exchange control traffic only.
        """

        self._has_packet_in_buffer = has_packet_in_buffer
        """
        True if the node has a packet in the buffer ready to send, False otherwise.
        """

        self._link_capacity = link_capacity
        """
        Max bandwidth available on the link connecting to the node.
        """

        self._id = id
        """
        Unique identifier of the node.
        Can be a global identifier or a neighbour identifier, according to the needs.
        """

        self._neighbours = []
        """
        List of state of neighbour nodes.
        Each neighbour is represented by a StateBean object.
        Different properties can be set according if the state bean represents
        the node owning the agent or a neighbour.
        """

    @property
    def energy(self):
        return self._energy
    
    @property
    def power_state(self):
        return self._power_state
    
    @property
    def has_packet_in_buffer(self):
        return self._has_packet_in_buffer
    
    @property
    def link_capacity(self):
        return self._link_capacity
    
    @property
    def id(self):
        return self._id
    
    def add_neighbour(self, neighbour):
        self._neighbours.append(neighbour)
    
    """
    Returns a list of tensors representing the state of the agent.
    Each tensor is a state variable.
    """
    def to_tensor(self):
        
        # NOTE: keep in sync with observation_spec in AgentFacade constructor
        #      and with the state variables in the constructor
        return [
            tf.constant(
                name="node_energy",
                value=self.energy,
                shape=(1),
                dtype=tf.float32
            ),
            tf.constant(
                name="node_power_state",
                value=self.power_state,
                shape=(1),
                dtype=tf.int32
            ),
            tf.constant(
                name="node_has_packet_in_buffer",
                value=self.has_packet_in_buffer,
                shape=(1),
                dtype=tf.bool
            ),
            tf.constant(
                name="node_link_capacity",
                value=self.link_capacity,
                shape=(1),
                dtype=tf.float32
            ),
            tf.constant(
                name="node_id",
                value=self.id,
                shape=(1),
                dtype=tf.string
            ),

            # list of neighbour states is converted to a list of tensors by
            # recursively calling to_tensor() on each neighbour
            [n.to_tensor() for n in self._neighbours]
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

