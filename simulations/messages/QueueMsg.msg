cplusplus(h){{
#include <string>

#define QUEUE_MSG_TOPIC "queue_state_topic"
#define is_queue_msg(_msg) strcmp(_msg->getName(), QUEUE_MSG_TOPIC) == 0

}}

enum QueueMsgKind{
    
    QUEUE_STATE_UPDATE = 1;
    QUEUE_DATA_REQUEST = 2;
    QUEUE_DATA_RESPONSE = 3;
}

message QueueMsg extends cMessage{   
    
}

cplusplus(QueueMsg::QueueMsg){{
    this->setName(QUEUE_MSG_TOPIC); 
}}