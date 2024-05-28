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
            print("called train in replay buffered agent")
            self._train_counter += 1
            values_batched = tf.nest.map_structure(lambda t: tf.stack([t] * self._batch_size), experience)
            self._replay_buffer.add_batch(values_batched)
            
            loss_info = None
            if self._train_counter % self._train_frequency == 0:
                dataset = self._replay_buffer.as_dataset(sample_batch_size=sample_batch_size, num_steps=num_steps)
                iterator = iter(dataset)
                for _ in range(1):
                    t, _ = next(iterator)
                    print("EXPERIENCE FOR TRAINING" + str(t))
                    loss_info = super().train(experience=t)
                    print("LOSS" + str(loss_info.loss))
                #print q values
                print("Q_VALUES" + str(self._q_network(t.observation)))
            return loss_info
        
    return ReplayBufferedAgent

class AgentEnum(Enum):
    RANDOM_AGENT = 1
    DQN_AGENT = 2

class LossEnum(Enum):
    SQUARED_LOSS = 1
    HUBER_LOSS = 2

    def get_loss_fn(self):
        match self:
            case LossEnum.SQUARED_LOSS:
                return common.element_wise_squared_loss
            case LossEnum.HUBER_LOSS:
                return common.element_wise_huber_loss

class OptimizerEnum(Enum):
    ADAM = 1
    SGD = 2
    RSMPROP = 3
    ADAGRAD = 4
    
class ExplorationEnum(Enum):
    EPSILON_GREEDY = 1
    BOLTZMANN = 2

class ActivationEnum(Enum):
    RELU = 1
    TANH = 2
    SIGMOID = 3
    LINEAR = 4
    SOFTMAX = 5

    def get_activation_layer(self):
        match self:
            case ActivationEnum.RELU:
                return tf.keras.activations.relu
            case ActivationEnum.TANH:
                return tf.keras.activations.tanh
            case ActivationEnum.SIGMOID:
                return tf.keras.activations.sigmoid
            case ActivationEnum.LINEAR:
                return tf.keras.activations.linear
            case ActivationEnum.SOFTMAX:
                return tf.keras.activations.softmax
class AgentFactory():
    
    @staticmethod
    def create_agent(agent_description, time_step_spec, action_spec, observation_and_action_constraint_splitter = None):
        agent_type = agent_description["agent_type"]
        match agent_type:
            case AgentEnum.RANDOM_AGENT:
                return random_agent.RandomAgent(time_step_spec, action_spec)
            case AgentEnum.DQN_AGENT:
                num_actions = action_spec.maximum - action_spec.minimum + 1
                # Agent parameters
                fc_layer_params = agent_description["fc_layer_params"]
                learning_rate = agent_description["learning_rate"]
                activation_layer = agent_description["activation_layer"]
                eps_greedy_bolz = agent_description["eps_greedy_bolz_choose"]
                epsilon_greedy_value = agent_description["epsilon_greedy"]
                boltzmann_temperature_value = agent_description["boltzmann_temperature"]
                error_loss_fn = agent_description["error_loss_fn"]
                optimizer = agent_description["optimizer"]
                gamma = agent_description["gamma"]
                rsmprop_momentum = agent_description["rsmprop_momentum"]
                rsmprop_rho = agent_description["rsmprop_rho"]
                adam_beta_1 = agent_description["adam_beta_1"]
                adam_beta_2 = agent_description["adam_beta_2"]
                adagrad_initial_accumulator_value = agent_description["adagrad_initial_accumulator_value"]
                # Replay buffer parameters
                rb_max__length = agent_description["rb_max_length"]
                rb_batch_size = agent_description["rb_batch_size"]
                rb_train_freq = agent_description["rb_train_freq"]
                rb_sample_batch_size = agent_description["rb_sample_batch_size"]


                fc_layer_params = tuple(fc_layer_params) + (num_actions,)
                train_step_counter = tf.Variable(0)
                

                match optimizer:
                    case OptimizerEnum.SGD:
                        optimizer = tf.keras.optimizers.SGD(learning_rate=learning_rate, weight_decay=10)
                    case OptimizerEnum.RSMPROP:
                        optimizer = tf.keras.optimizers.RMSprop(learning_rate=learning_rate, momentum=rsmprop_momentum, rho=rsmprop_rho)
                    case OptimizerEnum.ADAM:
                        optimizer = tf.keras.optimizers.Adam(learning_rate=learning_rate, beta_1=adam_beta_1, beta_2=adam_beta_2)
                    case OptimizerEnum.ADAGRAD:
                        optimizer = tf.keras.optimizers.Adagrad(learning_rate=learning_rate, initial_accumulator_value=adagrad_initial_accumulator_value)

                q_encoding_net = encoding_network.EncodingNetwork(
                    input_tensor_spec=time_step_spec.observation,
                    preprocessing_layers=#(
                        # if tuple, put one layer per tensor input (ES: 3 state tensor => 3 flatten layers)
                        # if there is only one state tensor as input, put only one layer without enclosing it in a tuple
                        tf.keras.layers.Flatten(),
                    #),
                    preprocessing_combiner=tf.keras.layers.Concatenate(axis=-1),
                    fc_layer_params=fc_layer_params,
                    activation_fn=activation_layer.get_activation_layer(),
                    kernel_initializer=tf.keras.initializers.VarianceScaling(
                        scale=2.0, mode='fan_in', distribution='truncated_normal', seed=3)
                )
                

                # creates a dqn agent decorated with a replay buffer
                ReplayBufferedDQNAgent = with_replay_buffer(
                    dqn_agent.DqnAgent,
                    sample_batch_size=rb_sample_batch_size,
                    num_steps=2,
                    train_frequency=rb_train_freq,
                    replay_buffer_class=tf_uniform_replay_buffer.TFUniformReplayBuffer,
                    batch_size=rb_batch_size,
                    max_length=rb_max__length
                )
                if eps_greedy_bolz == ExplorationEnum.EPSILON_GREEDY:
                    epsilon_greedy = epsilon_greedy_value
                    boltzmann_temperature = None
                else:
                    epsilon_greedy = None
                    boltzmann_temperature = boltzmann_temperature_value

                agent = ReplayBufferedDQNAgent(
                    time_step_spec,
                    action_spec,
                    q_network=q_encoding_net,
                    optimizer=optimizer,
                    td_errors_loss_fn=error_loss_fn.get_loss_fn(),
                    train_step_counter=train_step_counter,
                    epsilon_greedy=epsilon_greedy,
                    boltzmann_temperature=boltzmann_temperature,
                    gamma=gamma
                )        
                agent.initialize()
                return agent

            case _:
                raise Exception("Agent type not supported")