import SimulationMsg;


cplusplus(DataMsg::DataMsg){{
    this->setKind((int)SimulationMsgType::DATA);
}}

message DataMsg extends SimulationMsg{
    float data;
	int message_id;
};