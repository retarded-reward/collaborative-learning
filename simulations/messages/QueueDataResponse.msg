import QueueMsg;
import DataMsg;

cplusplus(QueueDataResponse::QueueDataResponse){{
    this->setKind((int)QueueMsgKind::QUEUE_DATA_RESPONSE);
}}

message QueueDataResponse extends QueueMsg{
    DataMsg data;
};
