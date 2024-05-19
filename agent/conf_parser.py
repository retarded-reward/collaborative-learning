import re
from agent_factory import AgentEnum
from agent_factory import LossEnum
from agent_factory import OptimizerEnum
from agent_factory import ExplorationEnum
from agent_factory import ActivationEnum

class ConfParser:

    pattern_random_agent = re.compile("random_agent|randomagent|random|random-agent")
    pattern_dqn_agent = re.compile("dqn_agent|dqnagent|dqn|dqn-agent")
    pattern_epsilon_greedy = re.compile("epsilon_greedy|epsilon-greedy|epsilon|greedy|epsilongreedy|eps_greedy|eps-greedy|eps|greedy|epsilon-greedy-bolz-choose|epsilon_greedy_bolz_choose|eps_greedy_bolz_choose|eps-greedy-bolz-choose|eps-greedy-bolz|eps_greedy_bolz|eps-greedy-bolz")
    pattern_boltzmann_temperature = re.compile("boltzmann_temperature|boltzmann-temperature|boltzmann|temperature|boltzmanntemperature")
    pattern_squared_loss = re.compile("squared_loss|squaredloss|squared|loss")
    pattern_huber_loss = re.compile("huber_loss|huberloss|huber|loss")
    pattern_adam_optimizer = re.compile("adam|adam_optimizer|adamoptimizer")
    pattern_sgd_optimizer = re.compile("sgd|sgd_optimizer|sgdoptimizer")
    pattern_rsmprop_optimizer = re.compile("rmsprop|rmsprop_optimizer|rmspropoptimizer")
    pattern_adagrad_optimizer = re.compile("adagrad|adagrad_optimizer|adagradoptimizer")
    pattern_relu = re.compile("relu")
    pattern_tanh = re.compile("tanh")
    pattern_sigmoid = re.compile("sigmoid")
    pattern_linear = re.compile("linear")
    pattern_softmax = re.compile("softmax")

    @staticmethod
    def parse_agent_from_json(json):
        ret = {}
        agent_type_raw = json["agent_type"].lower()
        if agent_type_raw != None:
            if ConfParser.pattern_random_agent.match(agent_type_raw):
                json["agent_type"] = AgentEnum.RANDOM_AGENT
            elif ConfParser.pattern_dqn_agent.match(agent_type_raw):
                json["agent_type"] = AgentEnum.DQN_AGENT

        epsilon_greedy = json.get("eps_greedy_bolz_choose")
        if epsilon_greedy != None:
            if ConfParser.pattern_epsilon_greedy.match(epsilon_greedy):
                json["eps_greedy_bolz_choose"] = ExplorationEnum.EPSILON_GREEDY
            else:
                json["eps_greedy_bolz_choose"] = ExplorationEnum.BOLTZMANN

        error_loss_fn = json.get("error_loss_fn")
        if error_loss_fn != None:
            if ConfParser.pattern_squared_loss.match(error_loss_fn):
                json["error_loss_fn"] = LossEnum.SQUARED_LOSS
            elif ConfParser.pattern_huber_loss.match(error_loss_fn):
                json["error_loss_fn"] = LossEnum.HUBER_LOSS

        optimizer = json.get("optimizer")
        if optimizer != None:
            if ConfParser.pattern_adam_optimizer.match(optimizer):
                json["optimizer"] = OptimizerEnum.ADAM
            elif ConfParser.pattern_sgd_optimizer.match(optimizer):
                json["optimizer"] = OptimizerEnum.SGD
            elif ConfParser.pattern_rsmprop_optimizer.match(optimizer):
                json["optimizer"] = OptimizerEnum.RSMPROP
            elif ConfParser.pattern_adagrad_optimizer.match(optimizer):
                json["optimizer"] = OptimizerEnum.ADAGRAD

        activation_layer = json.get("activation_layer")
        if activation_layer != None:
            if ConfParser.pattern_relu.match(activation_layer):
                json["activation_layer"] = ActivationEnum.RELU
            elif ConfParser.pattern_tanh.match(activation_layer):
                json["activation_layer"] = ActivationEnum.TANH
            elif ConfParser.pattern_sigmoid.match(activation_layer):
                json["activation_layer"] = ActivationEnum.SIGMOID
            elif ConfParser.pattern_linear.match(activation_layer):
                json["activation_layer"] = ActivationEnum.LINEAR
            elif ConfParser.pattern_softmax.match(activation_layer):
                json["activation_layer"] = ActivationEnum.SOFTMAX
        return json