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

class AgentFacade():
    
    
    class UnrewardedExperience():

        def __init__(self, state, action, message_id):
            self.state = state
            self.action = action
            self.message_id = message_id

    def __init__(self, max_neighbours = 10, agent_description = {"agent_type": AgentEnum.RANDOM_AGENT}, train_frequency = 2):
        self._last_experience = None
        self._max_neighbours = max_neighbours
        self._train_frequency = train_frequency
        self._train_counter = 0
        node_spec = tensor_spec.BoundedTensorSpec(shape=(1, 3), dtype=np.float32, minimum=[[0, 0, 0]], maximum=[1, 1, 1], name = "state")

        observation_spec = node_spec

        action_spec_root = tensor_spec.BoundedTensorSpec(shape=(), dtype=tf.int32, minimum=0, maximum=2, name = "choose_action")

        time_step_spec = ts.time_step_spec(observation_spec)

        # TODO: make the agent implementation easily configurable
        agent_root = AgentFactory.create_agent(
            agent_description = agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_root)
        
        agent_root.initialize()
        
        '''
        agent_change_power_state = AgentFactory.create_agent(
            agent_description = agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_change_power_state)
        
        agent_send_message = AgentFactory.create_agent(
            agent_description = agent_description, 
            time_step_spec = time_step_spec,
            action_spec = action_spec_send_message)
        '''
        
        self._root = DecisionTreeConsultant(agent_root, "root")
        #chang_power_state_node = DecisionTreeConsultant(agent_change_power_state, "change_power_state")
        #send_message_node =DecisionTreeConsultant(agent_send_message, "send_message")
        #self._root.add_choice(chang_power_state_node)
        #self._root.add_choice(send_message_node)
    

    def get_action(self, state_bean, rewards_bean):
        logging.info("Getting action for state: " + str(state_bean))
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
        time_step = state.to_tensor()
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
    agent_desc = ConfParser.parse_agent_from_json(json.load(open("agent/agent_conf.json")))
    agent = AgentFacade(
        agent_description=agent_desc)
    state = StateBean(energy_level=1, queue_state=[1], charge_rate=0)
    file = open("log.csv", "a")
    #elimino il contenuto del file
    file.truncate(0)
    #metto le colonne
    file.write("energy_level,queue_state,charge_rate,send_message,power_source,reward\n")
    reward = RewardBean(0)
    action = agent.get_action(state, None)
    #print("Azione scelta: " + str(action))
    for i in range(2000):
        
        random_energy = np.random.randint(0, 2)
        random_queue = 0
        random_charge = 0
        state = StateBean(energy_level=random_energy, queue_state=[random_queue], charge_rate=random_charge)
        action = agent.get_action(state, reward)
        #print("Azione scelta: " + str(action))
        if(state.energy_level >= 0.5 and action.send_message == ActionBean.SendEnum.SEND_MESSAGE):
            reward = RewardBean(1)
        elif(state.energy_level < 0.5 and action.send_message == ActionBean.SendEnum.DO_NOTHING):
            reward = RewardBean(1)
        else:
            reward = RewardBean(-1)
        #creo un log in un file csv in cui segno lo stato e l'azione scelta e la reward ricevuta
        file.write(str(state.energy_level) + "," + str(state.queue_state) + "," + str(state.charge_rate) + "," + str(action.send_message) + "," + str(action.power_source) + "," + str(reward.reward) + "\n")
    '''
    import pandas as pd

    df = pd.read_csv('log.csv')
    print(df)
    #prendo solo la colonna della reward
    reward = df['reward']

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
    plt.show()
    '''
    