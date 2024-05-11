from enum import Enum
from tf_agents.agents.random import fixed_policy_agent, random_agent
from typing import Callable, Union
from tf_agents.agents.dqn import dqn_agent
from tf_agents.utils import common
import tensorflow as tf
from tf_agents.networks import sequential, nest_map, q_network, mask_splitter_network, encoding_network
import keras
from tf_agents.specs import tensor_spec


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
                fc_layer_params = (100, 50)
                learning_rate = 1e-3
                train_step_counter = tf.Variable(0)

                #dense_layers = [AgentFactory._dense_layer(num_units) for num_units in fc_layer_params]
                dense_layers = [
                    tf.keras.layers.Flatten(input_shape=[3, 1]),
                    AgentFactory._dense_layer(100),
                    AgentFactory._dense_layer(50),
                    AgentFactory._dense_layer(20)
                ]
                print("FIRST DENSE LAYER: " + str(dense_layers[0].get_config()))
                q_values_layer = tf.keras.layers.Dense(
                    action_spec.maximum - action_spec.minimum + 1,
                    activation=None,
                    kernel_initializer=tf.keras.initializers.RandomUniform(
                        minval=-0.03, maxval=0.03),
                    bias_initializer=tf.keras.initializers.Constant(-0.2))
               
                q_net = sequential.Sequential(dense_layers + [q_values_layer], input_spec= time_step_spec.observation)
                
                #q_net.create_variables(
                #    input_tensor_spec= time_step_spec.observation)
               
                #q_net2 = nest_map.NestMap(dense_layers + [q_values_layer], input_spec= time_step_spec.observation, name="net2")
                '''                t = tf.constant([1])
                paddings = tf.constant([[0, 9]])
                #shape = tf.pad(time_step_spec.observation, paddings, "CONSTANT")
                shape = [tensor_spec.TensorSpec(shape=(10,), dtype=tf.float32, name='id'), 
                 tensor_spec.TensorSpec(shape=(10,), dtype=tf.int32, name='energy_state'), 
                 tensor_spec.TensorSpec(shape=(10,), dtype=tf.bool, name='power_state'), 
                 tensor_spec.TensorSpec(shape=(10,), dtype=tf.bool, name='has_packet_in_buffer'), 
                 tensor_spec.TensorSpec(shape=(10, 10), dtype=tf.float32, name='id_neighbour'), 
                 tensor_spec.TensorSpec(shape=(10, 10), dtype=tf.int32, name='energy_state_neighbour'), 
                 tensor_spec.TensorSpec(shape=(10, 10), dtype=tf.float32, name='link_capacity')]
                print("PADDED TENSOR: " + str(shape))
                ts = time_step_spec.observation
                for i in ts:
                    if i.shape == (1,):
                        i = tf.pad(i, paddings, "CONSTANT")
                input_shape = (10,)
                ishape =[input_shape for _ in range(10)]
                print(time_step_spec.observation)
                # 'constant_values' is 0.
                # rank of 't' is 2.
                tf.pad(t, paddings, "CONSTANT")  # [[0, 0, 0, 0, 0, 0, 0],
                                                #  [0, 0, 1, 2, 3, 0, 0],
                                                #  [0, 0, 4, 5, 6, 0, 0],
                                                #  [0, 0, 0, 0, 0, 0, 0]]
                print("PADDED TENSOR: " + str(tf.pad(t, paddings, "CONSTANT")))

                '''
                optimizer = tf.keras.optimizers.Adam(learning_rate=learning_rate)
                '''
                q_net = q_network.QNetwork(
                    action_spec=action_spec, 
                    input_tensor_spec=time_step_spec.observation, 
                    fc_layer_params=fc_layer_params)
                '''
                q_encoding_net = encoding_network.EncodingNetwork(
                    input_tensor_spec=time_step_spec.observation,
                    preprocessing_layers=(
                        tf.keras.layers.Flatten(),
                        tf.keras.layers.Flatten(),#input_shape=[2]),
                        tf.keras.layers.Flatten()
                    ),
                    preprocessing_combiner=tf.keras.layers.Concatenate(axis=-1),
                    fc_layer_params=(100, 50, 20, action_spec.maximum - action_spec.minimum + 1),
                    activation_fn=tf.keras.activations.relu,
                    kernel_initializer=tf.keras.initializers.VarianceScaling(
                        scale=2.0, mode='fan_in', distribution='truncated_normal')
                )
                


                q_net_masked = mask_splitter_network.MaskSplitterNetwork(
                    splitter_fn=AgentFactory._splitter_fn,
                    wrapped_network=q_net,
                    passthrough_mask=True
                )
                
                return dqn_agent.DqnAgent(
                    time_step_spec,
                    action_spec,
                    q_network=q_encoding_net,
                    optimizer=optimizer,
                    td_errors_loss_fn=common.element_wise_squared_loss,
                    train_step_counter=train_step_counter,
                    epsilon_greedy=0.01)

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
