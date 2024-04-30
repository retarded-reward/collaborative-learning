import QueueMsg;

cplusplus(h){{
#include "units.h"
#include <cstddef>
}}

cplusplus(QueueStateUpdate::QueueStateUpdate){{
    this->setKind((int)QueueMsgKind::QUEUE_STATE_UPDATE);
}}

message QueueStateUpdate extends QueueMsg{
    // percentage of queue buffer occupied by data
    float buffer_pop_percentage @cppType(percentage_t);
    
    // how many packets have been dropped since last update
    int num_of_dropped;
};
