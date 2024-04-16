import SimulationMsg;


cplusplus(DataMsg::DataMsg){{
    this->setKind((int)SimulationMsgKind::DATA_MSG);
}}

message DataMsg extends SimulationMsg{
    float data;
	int msg_id;
};