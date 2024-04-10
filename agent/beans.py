# TODO: maybe is better to move creation of *_spec objects in static methods
#       of the beans. For example, the StateBean class knows what is the structure
#       of the state tensor, so it can provide a method that returns the observation_spec.

from enum import IntEnum

import tensorflow as tf

class NodePowerState(IntEnum):
    OFF = 0
    ON = 1

class StateBean():

    def __init__(self,
        energy : float = 0, 
        power_state : NodePowerState = False,
        has_packet_in_buffer : bool = False,
        link_capacity : float = -1, 
        id : float = -1):
        
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
        Unique identifier of the node, if available/needed/meaningful.
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
    
    @energy.setter
    def energy(self, energy):
        self._energy = energy
    
    @property
    def power_state(self):
        return self._power_state
    
    @power_state.setter
    def power_state(self, power_state):
        self._power_state = power_state
    
    @property
    def has_packet_in_buffer(self):
        return self._has_packet_in_buffer
    
    @has_packet_in_buffer.setter
    def has_packet_in_buffer(self, has_packet_in_buffer):
        self._has_packet_in_buffer = has_packet_in_buffer
    
    @property
    def link_capacity(self):
        return self._link_capacity
    
    @link_capacity.setter
    def link_capacity(self, link_capacity):
        self._link_capacity = link_capacity
    
    @property
    def id(self):
        return self._id
    
    @id.setter
    def id(self, id):
        self._id = id
    
    def add_neighbour(self, neighbour):
        self._neighbours.append(neighbour)
    
    """
    Returns a list of tensors representing the state of the agent.
    Each tensor is a state variable.
    """
    def to_tensor(self, max_neighbours):
        
        # NOTE: keep in sync with observation_spec in AgentFacade constructor
        #      and with the state variables in the constructor

        node_spec = [
            tf.constant(shape=(1), dtype=tf.float32, name = "id", value=self.id),
            tf.constant(shape=(1), dtype=tf.int32, name = "energy_state", value=self.energy),
            tf.constant(shape=(1), dtype=tf.bool, name = "power_state", value=self.power_state),
            tf.constant(shape=(1), dtype=tf.bool, name = "has_packet_in_buffer", value=self.has_packet_in_buffer)
        ]
        ngs = {"id_neighbour": [], "energy_state_neighbour": [], "link_capacity": []}
        for i in range(max_neighbours):
            if i >= len(self._neighbours):
                ngs["id_neighbour"].append(-1)
                ngs["energy_state_neighbour"].append(0)
                ngs["link_capacity"].append(0)
            else:
                neighbour = self._neighbours[i] 
                ngs["id_neighbour"].append(neighbour.id)
                ngs["energy_state_neighbour"].append(neighbour.energy)
                ngs["link_capacity"].append(neighbour.link_capacity)

        
        node_spec.append(tf.constant(shape=(max_neighbours), dtype=tf.float32, name = "id_neighbour", value=ngs["id_neighbour"]))
        node_spec.append(tf.constant(shape=(max_neighbours), dtype=tf.int32, name = "energy_state_neighbour", value=ngs["energy_state_neighbour"]))
        node_spec.append(tf.constant(shape=(max_neighbours), dtype=tf.float32, name = "link_capacity", value=ngs["link_capacity"]))
            
        return node_spec
    
    
    
    def __str__(self):
        return "StateBean(energy={}, power_state={}, has_packet_in_buffer={}, link_capacity={}, id={}, neighbours={})".format(
            self.energy, self.power_state, self.has_packet_in_buffer, self.link_capacity, self.id, self._neighbours)
    
class RewardBean():

    def __init__(self, message_id : int = None, reward : int = None):
        # id of the message corresponding to the reward
        self._message_id = message_id
        self._reward = reward

    @property
    def message_id(self):
        return self._message_id
    
    @message_id.setter
    def message_id(self, message_id):
        self._message_id = message_id
    
    @property
    def reward(self):
        return self._reward
    
    @reward.setter
    def reward(self, reward):
        self._reward = reward
    
class RewardsBean():
    
    def __init__(self, rewards : [RewardBean] = []):
        self._rewards = rewards

    @property
    def rewards(self):
        return self._rewards
    
    def add_reward(self, msg_id : int, reward : int):
        self._rewards.append(RewardBean(message_id=msg_id, reward=reward))

class RelayBean():

    def __init__(self):
        self._neighbour_id = -1
        self._rate = 0.0
        @property
        def neighbour_id(self):
            return self._neighbour_id
        
        @neighbour_id.setter
        def neighbour_id(self, neighbour_id: int):
            self._neighbour_id = neighbour_id
        
        @property
        def rate(self):
            return self._rate
        
        @rate.setter
        def rate(self, rate: float):
            self._rate = rate

# TODO: implement the other actions according to the model
class ActionBean():
    
    def __init__(self):
        self._changePowerState = -1
        self.relaySet = []

    @property
    def send_message(self):
        return self.changePowerState
    
    @property
    def changePowerState(self):
        return self._changePowerState
    
    @changePowerState.setter
    def changePowerState(self, changePowerState):
        self._changePowerState = changePowerState
    
    def add_relay(self, relay: RelayBean):
        self.relaySet.append(relay)
    
    def get_relay(self, index: int) -> RelayBean:
        return self.relaySet[index]
    
    def get_relay_count(self) -> int:
        return len(self.relaySet)

