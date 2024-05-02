import SimulationMsg;

cplusplus(h){{
    #include "units.h"
}}

cplusplus(RewardMsg::RewardMsg){{
    this->setKind((int)SimulationMsgKind::REWARD_MSG);
}}

message RewardMsg extends SimulationMsg{
    float value @cppType(reward_t);
}
