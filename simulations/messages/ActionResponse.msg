import AgentClientMsg;

enum SelectPowerSource{
    BATTERY = 1;
    POWER_CHORD = 2;
}

cplusplus(ActionResponse::ActionResponse){{
    this->setKind((int)AgentClientMsgKind::ACTION_RESPONSE);
}}

message ActionResponse extends AgentClientMsg{
    bool send_message;
    SelectPowerSource select_power_source;
    
    // selects the queue to pick the msg to send
    int queue;

    int msg_to_send = 1; //Change it to make agent support multisend
};

