import SimulationMsg;

cplusplus(h){{
#include "units.h"
}}

cplusplus(DataMsg::DataMsg){{
    this->setKind((int)SimulationMsgKind::DATA_MSG);
}}

message DataMsg extends SimulationMsg{
    float data @cppType(B_t);
    simtime_t queueing_time;
};