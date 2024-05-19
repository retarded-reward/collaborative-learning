from typing import Optional, Text

import numpy as np
import tensorflow as tf
from tf_agents.agents.random import fixed_policy_agent, random_agent
from tf_agents.agents.tf_agent import LossInfo, TFAgent
from tf_agents.policies import random_tf_policy
from tf_agents.specs import array_spec, tensor_spec
from tf_agents.trajectories import time_step as ts
from tf_agents.trajectories import trajectory
from tf_agents.trajectories.time_step import StepType, TimeStep
from tf_agents.typing import types
from tf_agents.utils import common
from decisions import DecisionTreeConsultant
from decisions import Experience
from typing import Callable, Union
from tf_agents.trajectories.policy_step import PolicyStep
from beans import ActionBean, RewardBean, StateBean

from agent_factory import AgentEnum, AgentFactory
from conf_parser import ConfParser
import json
import os
import logging

logging.root.setLevel(logging.DEBUG)

class StubbornAgent():
    """
    A constant agent that always returns the same action.
    """

    def __init__(self, action: int):
        self._action = PolicyStep(action)
        self.policy = self
        self.collect_policy = self.policy

    def action(self, time_step: TimeStep):
        return self._action
    
    def train(self, experience: types.TimeStep):
        # too stubborn to learn
        pass

class AgentFacadeBean():

    def __init__(self, n_queues = 1, agent_description_path = None,
     agent_description = None):
        self.n_queues = n_queues
        self.agent_description_path = agent_description_path
        self.agent_description = agent_description

class AgentFacade():

    def _init_agent_config(self, agent_description_path: str, agent_description):
        if(agent_description_path == None):
            agent_description_path = os.environ.get('AGENT_PATH') + "/agent_conf.json"
        if(agent_description == None):
            self._agent_description = ConfParser.parse_agent_from_json(json.load(open(agent_description_path)))

    def _init_specs(self):

        self._observation_spec = StateBean.observation_spec(self._n_queues)

        self._action_spec_root = tensor_spec.BoundedTensorSpec(
            shape=(),
            dtype=tf.int32, 
            minimum=0,
            maximum=1, 
            name = "choose_action")
        
        self._action_spec_choose_queue = tensor_spec.BoundedTensorSpec(
            shape = (),
            dtype=tf.int32,
            minimum=0,
            maximum=self._n_queues - 1,
            name="choose_queue"
        )

        self._action_spec_choose_power_source = tensor_spec.BoundedTensorSpec(
            shape=(),
            dtype=tf.int32, 
            minimum=0, 
            maximum=1, 
            name = "choose_power_source"
        )

        self._time_step_spec = ts.time_step_spec(self._observation_spec)
    
    def _init_decision_tree(self) -> DecisionTreeConsultant:
        
        agent_root = AgentFactory.create_agent(
            agent_description = self._agent_description, 
            time_step_spec = self._time_step_spec,
            action_spec = self._action_spec_root)
        agent_queue = AgentFactory.create_agent(
            agent_description = self._agent_description, 
            time_step_spec = self._time_step_spec,
            action_spec = self._action_spec_choose_queue)
        agent_power_source = AgentFactory.create_agent(
            agent_description = self._agent_description, 
            time_step_spec = self._time_step_spec,
            action_spec = self._action_spec_choose_power_source)
        
        # root: {do_nothing, send_message}
        # send_message: {queue_0, queue_1, ...}
        # queue_i: {power_source_0, power_source_1}
        self._root = DecisionTreeConsultant(agent_root, "root")
        # since action values can be provided only by leaves of the tree, the root
        # must have a leaf for the do_nothing action. This is implemented as a
        # consultant wrapping a StubbornAgent that always returns 0 (can be ignored).        
        self._root.add_choice(DecisionTreeConsultant(StubbornAgent(0), "do_nothing"))
        queue_consultant = DecisionTreeConsultant(agent_queue, "choose_queue")
        self._root.add_choice(queue_consultant)
        for i in range(self._n_queues):
            queue_consultant.add_choice(DecisionTreeConsultant(
                agent_power_source, f"power_source_for_queue_{i}"))
    
    def __init__(self, bean: AgentFacadeBean):
        """
        Initializes the Agent object.

        Args:
            n_queues (int): Number of queues.
            agent_description_path (str): Path to the agent description JSON file.
            agent_description (dict): Agent description dictionary. If None, it will be parsed from the JSON file.

        Returns:
            None
        """
        #printo il path assoluto
        self._init_agent_config(bean.agent_description_path, bean.agent_description)

        self._last_experience = None
        self._n_queues = bean.n_queues
        
        self._init_specs()        
        
        self._init_decision_tree()
    

    def get_action(self, state_bean, rewards_bean):
        logging.debug("Getting action for state: " + str(state_bean))
        return self._get_action(state_bean, rewards_bean)
    
    def _get_action(self, state, reward):
        # updates agent policy using reward from previous action
        if(self._last_experience is not None):
            r = tf.constant(value=reward.reward, shape = (), dtype=tf.float32)
            exp = Experience(self._last_experience[0], self._last_experience[1], r)
            
            self._root.train([exp])

        
        
        # Computes TimeStep object by resetting the environment
        # at the given state
        # and uses it to get the action
        time_step = state.to_tensor(self._n_queues)
        action = []
        self._root.get_decisions(parent_state=time_step, decision_path=action)
        # Stores the last experience
        self._last_experience = (time_step, action)

        action_bean = self._decision_path_to_action_bean(action)
        logging.debug("Action: " + str(action_bean))
        return action_bean
    
    def _decision_path_to_action_bean(self, decision_path):
        action = int(decision_path[0].value.action)
        if action == int(ActionBean.SendEnum.DO_NOTHING):
            action_bean = ActionBean(send_message=ActionBean.SendEnum.DO_NOTHING)
            action_bean.power_source = ActionBean.PowerSourceEnum.NO_SOURCE
            action_bean.queue = -1
        else:
            action_bean = ActionBean(send_message=ActionBean.SendEnum.SEND_MESSAGE)
            selected_queue = int(decision_path[1].value.action)
            selected_power_source = int(decision_path[2].value.action)
            action_bean.power_source = ActionBean.PowerSourceEnum(selected_power_source)
            action_bean.queue = selected_queue
        return action_bean              
            
                  
            
# Test main
def test_plotting():
    
    tf.compat.v1.logging.set_verbosity(tf.compat.v1.logging.DEBUG)
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
    n_queues = 10
    agent_facade_bean = AgentFacadeBean(n_queues=n_queues)
    agent = AgentFacade(agent_facade_bean)
    state = StateBean(energy_level=1, queue_state=[1] * n_queues, charge_rate=0)
    file = open("tests/log.csv", "a")
    #elimino il contenuto del file
    file.truncate(0)
    #metto le colonne
    file.write("energy_level;queue_state;charge_rate;send_message;power_source;queue;reward\n")
    reward = RewardBean(0)
    action = agent.get_action(state, None)
    for i in range(10):
        
        random_energy = np.random.randint(0, 100) / float(100)
        random_queue = 0
        random_charge = 0
        state = StateBean(energy_level=random_energy, queue_state=[random_queue] * n_queues, charge_rate=random_charge)
        action = agent.get_action(state, reward)
        #print("Azione scelta: " + str(action))
        '''
        if(state.energy_level >= 0.5 and action.send_message == ActionBean.SendEnum.SEND_MESSAGE):
            reward = RewardBean(0)
        elif(state.energy_level < 0.5 and action.send_message == ActionBean.SendEnum.DO_NOTHING):
            reward = RewardBean(0)
        else:
            reward = RewardBean(-1)
        '''
        if(ActionBean.SendEnum.DO_NOTHING == action.send_message):
            reward = RewardBean(-1)
        else:
            reward = RewardBean(0)
        #creo un log in un file csv in cui segno lo stato e l'azione scelta e la reward ricevuta
        file.write(str(state.energy_level) + ";" + str(state.queue_state) + ";" + str(state.charge_rate) + ";" + str(action.send_message) + ";" + str(action.power_source) + ";" + str(action.queue) +";" + str(reward.reward) + "\n")
    
    file.close()
    
    import pandas as pd

    df = pd.read_csv('tests/log.csv', sep=';')
    reward = df['reward']
    cumulative_reward_df = pd.DataFrame()
    for i in range(0, len(reward)):
        cumulative_reward_df.at[i, 'cumulative_reward'] = reward[0:i].sum()

    #faccio un grafico
    import matplotlib.pyplot as plt
    plt.figure(figsize=(10, 5))
    plt.plot(cumulative_reward_df)
    plt.xlabel('Time step')
    plt.ylabel('Cumulative reward')
    plt.savefig('tests/cumulative_reward.png')
    plt.show()

def test_get_action():
    n_queues = 10
    agent_facade_bean = AgentFacadeBean(n_queues=n_queues)
    agent = AgentFacade(agent_facade_bean)
    state = StateBean(energy_level=1, queue_state=[1] * n_queues, charge_rate=0)
    reward = RewardBean(-1)
    for i in range(10):
        action = agent.get_action(state, reward)
        print("decision path: " + str(action))

if __name__ == '__main__':
    test_plotting()
    #test_get_action()
