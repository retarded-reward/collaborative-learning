import QueueMsg;

cplusplus(QueueDataRequest::QueueDataRequest){{
    this->setKind((int)QueueMsgKind::QUEUE_DATA_REQUEST);
}}

message QueueDataRequest extends QueueMsg{
    
    // how many msgs we are fetching from the queue.
    int data_n;
};
