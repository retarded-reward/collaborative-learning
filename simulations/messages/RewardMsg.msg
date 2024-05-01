import SimulationMsg;

cplusplus(h){{
    #include "units.h"
}}

cplusplus(RewardMsg::RewardMsg){{
    this->setKind((int)SimulationMsgKind::REWARD_MSG);
}}

message RewardMsg extends SimulationMsg{
    float value; //TODO BUG if it is set as reward_t it doesn't build caus says reward_t type is unknown
};
