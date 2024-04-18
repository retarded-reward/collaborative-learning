cplusplus(h){{
#include <string>

#define TIMEOUT_TOPIC "timeout"
#define is_timeout_msg(_msg) strcmp(_msg->getName(), TIMEOUT_TOPIC) == 0

}}

enum TimeoutKind {
    
    ASK_ACTION = 1;
}


// Controller control loop can use this messages to shcedule
// its activities
message Timeout{   
    simtime_t delta;    // Timeout will be delivered after delta sim time
}

cplusplus(Timeout){{
    public:
        Timeout(enum TimeoutKind timeout_kind, int delta){
            
            this->setKind(TimeoutKind::ASK_ACTION);
            this->delta = delta;
            this->setName(TIMEOUT_TOPIC);
        }
}}