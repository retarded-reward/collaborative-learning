import SinkRewardMsg;
import SimulationMsg;


cplusplus(Reward::Reward){{
    this->setKind((int)SimulationMsgKind::REWARD_MSG);
}}

message Reward extends SimulationMsg{
    SinkRewardMsg sinkReward;
};
