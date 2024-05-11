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

from beans import ActionBean, RewardBean, StateBean

from agent_factory import AgentEnum, AgentFactory
from conf_parser import ConfParser
import json
import os
import logging

logging.root.setLevel(logging.DEBUG)

class AgentFacade():

    def __init__(self, 
            n_queues = 1,
            agent_description_path = "agent/agent_conf.json",
            agent_description = None, 
            train_frequency = 2):
        """
        Initializes the Agent object.

        Args:
            n_queues (int): Number of queues.
            agent_description_path (str): Path to the agent description JSON file.
            agent_description (dict): Agent description dictionary. If None, it will be parsed from the JSON file.
            train_frequency (int): Frequency of training.

        Returns:
            None
        """
        self._last_experience = None
        self._n_queues = n_queues
        self._train_frequency = train_frequency
        self._train_counter = 0
        if(agent_description == None):
            self._agent_description = ConfParser.parse_agent_from_json(json.load(open(agent_description_path)))
        
        node_spec_multi_queue = (
            tensor_spec.BoundedTensorSpec(shape=[], dtype=np.float32, minimum=0, maximum=1, name = "energy_level"),
            tensor_spec.BoundedTensorSpec(shape=[n_queues], dtype=np.float32, minimum=[0] * n_queues, maximum=[1] * n_queues, name = "queue_state"),
            tensor_spec.BoundedTensorSpec(shape=[], dtype=np.float32, minimum=0, maximum=1, name = "charge_rate") 
        )
        observation_spec = node_spec_multi_queue

        action_spec_root = tensor_spec.BoundedTensorSpec(shape=(), dtype=tf.int32, minimum=0, maximum=2, name = "choose_action")

        time_step_spec = ts.time_step_spec(observation_spec)
        agent_root = AgentFactory.create_agent(
            agent_description = self._agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_root)
        
        agent_root.initialize()
        
        self._root = DecisionTreeConsultant(agent_root, "root")
    

    def get_action(self, state_bean, rewards_bean):
        logging.debug("Getting action for state: " + str(state_bean))
        return self._get_action(state_bean, rewards_bean)
    
    def _get_action(self, state, reward):
        # updates agent policy using reward from previous action
        if(self._last_experience is not None):
            #print("Reward: " + str(reward.reward))
            r = tf.constant(value=reward.reward, shape = (), dtype=tf.float32)
            exp = Experience(self._last_experience[0], self._last_experience[1], r)
            train_choose = False
            self._train_counter += 1
            if(self._train_counter == self._train_frequency):
                train_choose = True
                self._train_counter = 0
            
            self._root.train([exp], train = train_choose)

        
        
        # Computes TimeStep object by resetting the environment
        # at the given state
        # and uses it to get the action
        time_step = state.to_tensor(self._n_queues)
        action = []
        self._root.get_decisions(parent_state=time_step, decision_path=action)
        # Stores the last experience
        self._last_experience = (time_step, action)

        action_bean = self._decision_path_to_action_bean(action)
        #print("Action: " + str(action_bean))
        return action_bean
    
    def _decision_path_to_action_bean(self, decision_path):
        action = int(decision_path[0].value.action)
        match action:
            case 0:
                return ActionBean(send_message=ActionBean.SendEnum.DO_NOTHING)
            case 1:
                return ActionBean(
                    send_message=ActionBean.SendEnum.SEND_MESSAGE, 
                    power_source=ActionBean.PowerSourceEnum.BATTERY)
            case 2:
                return ActionBean(
                    send_message=ActionBean.SendEnum.SEND_MESSAGE, 
                    power_source=ActionBean.PowerSourceEnum.POWER_CHORD)
            
                  
            
# Test main
if __name__ == '__main__':
    tf.compat.v1.logging.set_verbosity(tf.compat.v1.logging.DEBUG)
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
    n_queues = 10
    agent = AgentFacade(
        n_queues=n_queues)
    state = StateBean(energy_level=1, queue_state=[1] * 20, charge_rate=0)
    file = open("log.csv", "a")
    #elimino il contenuto del file
    file.truncate(0)
    #metto le colonne
    file.write("energy_level;queue_state;charge_rate;send_message;power_source;reward\n")
    reward = RewardBean(0)
    action = agent.get_action(state, None)
    #print("Azione scelta: " + str(action))
    for i in range(10):
        
        random_energy = np.random.randint(0, 100) / float(100)
        random_queue = 0
        random_charge = 0
        state = StateBean(energy_level=random_energy, queue_state=[random_queue] * n_queues, charge_rate=random_charge)
        action = agent.get_action(state, reward)
        #print("Azione scelta: " + str(action))
        if(state.energy_level >= 0.5 and action.send_message == ActionBean.SendEnum.SEND_MESSAGE):
            reward = RewardBean(1)
        elif(state.energy_level < 0.5 and action.send_message == ActionBean.SendEnum.DO_NOTHING):
            reward = RewardBean(1)
        else:
            reward = RewardBean(-1)
        #creo un log in un file csv in cui segno lo stato e l'azione scelta e la reward ricevuta
        file.write(str(state.energy_level) + ";" + str(state.queue_state) + ";" + str(state.charge_rate) + ";" + str(action.send_message) + ";" + str(action.power_source) + ";" + str(reward.reward) + "\n")
    
    file.close()
    
    import pandas as pd

    df = pd.read_csv('log.csv', sep=';')
    #print(df)
    #prendo solo la colonna della reward
    reward = df['reward']
    #print(reward)

    #trasformo tutti i -1000 in -1
    #reward = reward.replace(-10000, -1)

    #faccio un dataframe con la reward comulativa per ogni istante
    cumulative_reward_df = pd.DataFrame()
    for i in range(0, len(reward)):
        cumulative_reward_df.at[i, 'cumulative_reward'] = reward[0:i].sum()

    #faccio un grafico
    import matplotlib.pyplot as plt
    plt.figure(figsize=(10, 5))
    plt.plot(cumulative_reward_df)
    plt.xlabel('Time step')
    plt.ylabel('Cumulative reward')
    plt.savefig('cumulative_reward.png')
    plt.show()

    
    
    