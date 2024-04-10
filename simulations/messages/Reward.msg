import SinkRewardMsg;
import SimulationMsg;


cplusplus(Reward::Reward){{
    this->setKind((int)SimulationMsgType::REWARD);
}}

message Reward extends SimulationMsg{
    SinkRewardMsg sinkReward;
};
