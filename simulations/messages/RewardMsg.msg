import SimulationMsg;


cplusplus(RewardMsg::RewardMsg){{
    this->setKind((int)SimulationMsgKind::REWARD_MSG);
}}

message RewardMsg extends SimulationMsg{
    float value;
};
