from __future__ import annotations

import tensorflow as tf

from tf_agents.typing.types import Tensor
from tf_agents.trajectories.trajectory import Trajectory
from tf_agents.trajectories import time_step
from tf_agents.trajectories.time_step import StepType
from tf_agents.trajectories.policy_step import PolicyStep
from tf_agents.agents.tf_agent import TFAgent
from tf_agents.replay_buffers import tf_uniform_replay_buffer

from typing import Callable, Iterable, List, Tuple

class Decision():
    """
    The result of a node in the decision tree
    """

    def __init__(self, name: str, value: PolicyStep):
        self._name = name
        """ Human readable name of decision """
        self._value = value
        """ The value of the decision """

    @property
    def name(self):
        return self._name
    
    @property
    def value(self):
        return self._value
    
    def __repr__(self):
        return f"Decision(name={self._name}, value={self._value})"
    

class Experience():

    def __init__(self, 
                 state : Tensor, 
                 decision_path : Iterable[Tuple[str, Tensor]], 
                 reward : int = None):
        self._state = state
        """ The state where the decision path led """
        self._decision_path = decision_path
        self._reward = reward

    @property
    def state(self):
        return self._state

    @property
    def decision_path(self):
        return self._decision_path

    @property
    def reward(self):
        return self._reward
    
    def __str__(self):
        return f"Experience(state={self._state}, decision_path={self._decision_path}, reward={self._reward})"

        
class DecisionTreeConsultant():
    """
    There can be multiple types of action that can be taken by an agent. An eterogeneous
    action space can be hard to implement for a single TF agent, expecially if
    an action can be picked from only one type at each time step. For example, Let's
    assume max three neighbours per node and float rates:

                                    root_decision
                                    (what to do?)
                                        / \
                                       /   \
                                      /     \
                    change_power_state       send_message
                example of action: (1)       example of action: [(1, 0, 1), (1.3, 0, 2)]
                             (Turn ON)       (forward msg to neighbour 1 and 3 with rate
                                              1.3 and 2 respectively)

    A DecisionTreeConsultant represent an ensemble of agents organized in a tree
    structure. Each node of the tree is a DecisionTreeConsultant and each decision is
    a branch of the tree.

    To know which type of action is more suitable, the DecisionTreeConsultant makes a
    a decision using its embedded agent. The decision is taken from multiple choices,
    with each choice representing another DecisionTreeConsultant, which can lead to a
    further branching of the tree or a leaf.

    A leaf DecisionTreeConsultant corresponds to a certain category of action. It has no
    choices and instead of making a decision, it asks its embedded agent the actual action
    to take among the ones available in the given action category. 
    """

    def __init__(self, agent: TFAgent,
        decision_name : str = "",
        deduce_consultant_state: Callable[[Tensor], Tensor] = lambda self, 
        parent_state: parent_state,
        deduce_consultant_experience: Callable[[Experience], Experience] = lambda self, 
        parent_reward: parent_reward):
        
        self._agent = agent
        """
        If the consultant is not a leaf of the tree, the value predicted by such agent
        is interpreted as the index of the selected choice. The agent must be
        careful to not predict values that aren't in available choices list bounds.

        The observation_spec of the agent must match the one of the state passed
        to the deduce_consultant_state method.
        """

        self._choices = []
        """
        List of child DecisionTreeConsultant.
        To make a decision, the consultant asks to pick a choice to its embedded agent.
        A child consultant is associated with each choice.
        If this consultant has no choices, instead of making a decision, it asks
        its embedded agent the action to take.
        """

        self._choices_name_to_index = {}
        """
        A map used internally to speed up choice lookup.
        Key: name of the decision
        Value: the index corresponding to the decision among the choices available to
        this consultant.
        """

        self._decision_name = decision_name
        """
        Used to reconstruct the decision tree path computed by the consultant.
        """

        self._deduce_consultant_state_impl = deduce_consultant_state
        """
        Specify an implementation in the constructor params if you want to use
        a refined state starting from the one passed by the parent.
        """

        self._deduce_consultant_experience_impl = deduce_consultant_experience
        """
        Specify an implementation in the constructor params if you want to use
        a refined experience starting from the one passed by the parent.
        """
            

    def _deduce_consultant_state(self, parent_state : Tensor) -> Tensor:
        return self._deduce_consultant_state_impl(self, parent_state)
    
    def _deduce_consultant_experience(self, parent_experience : Experience) -> Experience:
        return self._deduce_consultant_experience_impl(self, parent_experience)

    @property
    def decision_name(self):
        return self._decision_name

    def add_choice(self, child: DecisionTreeConsultant):
        """
        Adds a consultant as a choice for the current consultant.
        """
        self._choices.append(child)
        self._choices_name_to_index[child.decision_name] = len(self._choices) - 1


    def get_decisions(self, parent_state : Tensor,
        decision_path : List[Decision]):
        """
        Saves in the provided list the decision path traversed by the consultant.
        """
        decision_state = self._deduce_consultant_state(parent_state)
        ts = time_step.restart(decision_state)
        
        # uses embedded agent to compute decision and adds it as a deeper level
        # of the decision path
        decision = Decision(name=self.decision_name, value=self._agent.collect_policy.action(ts))
        decision_path.append(decision)

        # If this consultant has choices, it means that the decision path
        # must be further expanded in order to reach a leaf and get the
        # actual action to take.
        # We use the value of the decision to pick the next consultant
        if len(self._choices) > 0:
            next_consultant = self._choices[int(decision.value.action)]
            next_consultant.get_decisions(decision_state, decision_path)

    
    def train(self, experiences: Iterable[Experience], decision_path_level: int = 0):
        """
        Trains the agent using the given experiences.

        Args:
            experiences (Iterable[Experience]): The experiences to train the agent on.
            decision_path_level (int, optional): The level of the decision path to consider. Defaults to 0.
            train (bool, optional): Whether to perform training or not. Defaults to True.
        """
        #train=True
        for e in experiences:
        
            e = self._deduce_consultant_experience(e)
            decision = e.decision_path[decision_path_level]
            action = tf.constant(value=int(decision.value.action), shape=(), dtype=tf.int32)
            
            # trains embedded agent
            trajectory = Trajectory(
                step_type=tf.constant(value=1, shape=(), dtype=tf.int32),
                observation=self._deduce_consultant_state(e.state),
                action=action,
                policy_info=(),
                next_step_type=tf.constant(value=1, shape=(), dtype=tf.int32),
                reward=e.reward,
                discount=tf.constant(value=1, shape=(), dtype=tf.float32)
            )
            self._agent.train(experience=trajectory)
        
            # train all and only consultants that are part of the decision path.
            if len(self._choices) > 0:
                next_decision_path_level = decision_path_level + 1
                # extracts next Decision object from decision path
                next_decision_in_path = e.decision_path[next_decision_path_level]
                # Finds consultant choice using the Decision name and trains it
                next_consultant = self._choices[self._choices_name_to_index[next_decision_in_path.name]]
                next_consultant.train([e], next_decision_path_level)

# test the DecisionTreeConsultant class
if __name__ == "__main__":
    from tf_agents.agents.random.random_agent import RandomAgent
    from tf_agents.specs.tensor_spec import TensorSpec, BoundedTensorSpec
    
    def build_simple_decision_tree() -> DecisionTreeConsultant:
        
        time_step_spec = time_step.time_step_spec(
        observation_spec=TensorSpec(shape=(1,), dtype=tf.int32))
        NUM_OF_ROOT_CHOICES = 2

        root = DecisionTreeConsultant(
            agent=RandomAgent(
                time_step_spec = time_step_spec,
                action_spec = BoundedTensorSpec(
                    shape=(1), dtype=tf.int32, minimum=0, maximum=NUM_OF_ROOT_CHOICES - 1),),
            decision_name="root")
        root.add_choice(DecisionTreeConsultant(
            agent=RandomAgent(
                time_step_spec = time_step_spec,
                action_spec = TensorSpec(shape=(2), dtype=tf.int32)),
            decision_name="left_child"))
        root.add_choice(DecisionTreeConsultant(
            agent=RandomAgent(
                time_step_spec = time_step_spec,
                action_spec = TensorSpec(shape=(1), dtype=tf.float32)),
            decision_name="right_child"))

        return root
    
    def test_get_decisions_and_train():
        
        state = tf.constant([1], dtype=tf.int32)
        # two childs
        decision_path = []

        consultant = build_simple_decision_tree()
        
        consultant.get_decisions(state, decision_path)
        
        experiences = [
            Experience(
                state=state,
                decision_path=decision_path,
                reward=1),
            Experience(
                state=state,
                decision_path=decision_path,
                reward=2)
        ]

        consultant.train(experiences)
        decision_path.clear()
        consultant.get_decisions(state, decision_path)
        print("training completed:")
        print(str(decision_path))
    
    test_get_decisions_and_train() 


