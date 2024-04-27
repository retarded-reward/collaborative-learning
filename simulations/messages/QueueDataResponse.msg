import QueueMsg;
import DataMsg;
import QueueStateUpdate;

cplusplus(QueueDataResponse::QueueDataResponse){{
    this->setKind((int)QueueMsgKind::QUEUE_DATA_RESPONSE);
}}

message QueueDataResponse extends QueueMsg{
    
    // NOTE: actual size of fetched data array may differ from the num of data specified
    // in the corresponding queue data request
    DataMsg *data[] @owned;

    // Fetching data msgs from the queue arguably changes the queue state, so it's
    // reasonable to piggyback a queue state update in this queue data response
    QueueStateUpdate *stateUpdate @owned;
};
