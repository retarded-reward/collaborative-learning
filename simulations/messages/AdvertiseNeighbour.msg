import NodeStateMsg;
import SimulationMsg;


cplusplus(AdvertiseNeighbour::AdvertiseNeighbour){{
    this->setKind((int)SimulationMsgKind::ADVERTISEMENT_MSG);
}}

message AdvertiseNeighbour extends SimulationMsg{
    NodeStateMsg state;
};
