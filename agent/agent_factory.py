from enum import Enum
from tf_agents.agents.random import fixed_policy_agent, random_agent
from typing import Callable, Union
from tf_agents.agents.dqn import dqn_agent
from tf_agents.utils import common
import tensorflow as tf
from tf_agents.networks import sequential, nest_map, q_network, mask_splitter_network, encoding_network
import keras
from tf_agents.specs import tensor_spec
from tf_agents.trajectories.trajectory import Trajectory
from tf_agents.replay_buffers import tf_uniform_replay_buffer
from typing import Optional
from tf_agents.typing import types
from tf_agents.agents.tf_agent import LossInfo

def with_replay_buffer(tf_agent_class, sample_batch_size,
 num_steps, train_frequency, replay_buffer_class, *replay_buffer_ctor_args,
  **replay_buffer_ctor_kwargs):
    """
    Decorates a TF-Agents agent class with an enchanced train method that uses
    a replay buffer to store and sample experience.
    You can decorate any agent provided by TF-Agents. Isn't that exciting?!
    """
        
    class ReplayBufferedAgent(tf_agent_class):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._replay_buffer = replay_buffer_class(
                data_spec=self.collect_data_spec,
                *replay_buffer_ctor_args,
                **replay_buffer_ctor_kwargs)
            self._sample_batch_size = sample_batch_size
            self._num_steps = num_steps
            self._train_frequency = train_frequency
            self._train_counter = 0
            self._batch_size = replay_buffer_ctor_kwargs["batch_size"]

        def train(self, experience: Trajectory, weights: Optional[types.NestedTensor] = None, **kwargs) -> LossInfo:
            self._train_counter += 1
            values_batched = tf.nest.map_structure(lambda t: tf.stack([t] * self._batch_size), experience)
            self._replay_buffer.add_batch(values_batched)
            
            loss_info = LossInfo(loss = 0, extra = 0)
            if self._train_counter % self._train_frequency == 0:
                dataset = self._replay_buffer.as_dataset(sample_batch_size=sample_batch_size, num_steps=num_steps)
                iterator = iter(dataset)
                for _ in range(sample_batch_size):
                    t, _ = next(iterator)
                    loss_info = super().train(experience=t)
            
            return loss_info
        
    return ReplayBufferedAgent

class AgentEnum(Enum):
    RANDOM_AGENT = 1
    DQN_AGENT = 2

class AgentFactory():
    
    @staticmethod
    def create_agent(agent_description, time_step_spec, action_spec):
        agent_type = agent_description["agent_type"]
        match agent_type:
            case AgentEnum.RANDOM_AGENT:
                return random_agent.RandomAgent(time_step_spec, action_spec)
            case AgentEnum.DQN_AGENT:
                num_actions = action_spec.maximum - action_spec.minimum + 1
                fc_layer_params = (20, 10, num_actions)
                learning_rate = 1e-3
                train_step_counter = tf.Variable(0)
                
                optimizer = tf.keras.optimizers.Adam(learning_rate=learning_rate)

                q_encoding_net = encoding_network.EncodingNetwork(
                    input_tensor_spec=time_step_spec.observation,
                    preprocessing_layers=(
                        tf.keras.layers.Flatten(),
                        tf.keras.layers.Flatten(),#input_shape=[2]),
                        tf.keras.layers.Flatten(),
                    ),
                    preprocessing_combiner=tf.keras.layers.Concatenate(axis=-1),
                    fc_layer_params=fc_layer_params,
                    activation_fn=tf.keras.activations.relu,
                    kernel_initializer=tf.keras.initializers.VarianceScaling(
                        scale=2.0, mode='fan_in', distribution='truncated_normal')
                )
                

                # creates a dqn agent decorated with a replay buffer
                ReplayBufferedDQNAgent = with_replay_buffer(
                    dqn_agent.DqnAgent,
                    sample_batch_size=4,
                    num_steps=2,
                    train_frequency=2,
                    replay_buffer_class=tf_uniform_replay_buffer.TFUniformReplayBuffer,
                    batch_size=32,
                    max_length=1000
                )
                agent = ReplayBufferedDQNAgent(
                    time_step_spec,
                    action_spec,
                    q_network=q_encoding_net,
                    optimizer=optimizer,
                    td_errors_loss_fn=common.element_wise_squared_loss,
                    train_step_counter=train_step_counter,
                    epsilon_greedy=0.5
                )                
                agent.initialize()
                return agent

            case _:
                raise Exception("Agent type not supported")

    @staticmethod
    def _dense_layer(num_units):
        return tf.keras.layers.Dense(
            num_units,
            activation=tf.keras.activations.relu,
            kernel_initializer=tf.keras.initializers.VarianceScaling(
                scale=2.0, mode='fan_in', distribution='truncated_normal'))
    
    @staticmethod
    def _splitter_fn(observation):
        mask = [1, 1, 1]

        return observation, mask
