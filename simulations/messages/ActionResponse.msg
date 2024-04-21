import AgentClientMsg;

enum ChangePowerStateAction{
    DO_NOT_CHANGE = 2;
    TURN_ON = 1;
    TURN_OFF = 0;    
}

class ForwardDataAction{
    // id of chosen neighbour for the forwarded data
    int neighbour_id;

    // how much data must be sent to the neighbours in the relay set
    float rate;

    // TODO: scegliere se agente deve predire esattamente a quali vicini inviare
    // la R_neigh oppure sceglie solo numero di vicini da estrarre.

}

cplusplus(ActionResponse::ActionResponse){{
    this->setKind((int)AgentClientMsgKind::ACTION_RESPONSE);
}}

message ActionResponse extends AgentClientMsg{
    ChangePowerStateAction change_power_state;
    ForwardDataAction relay[];

};

