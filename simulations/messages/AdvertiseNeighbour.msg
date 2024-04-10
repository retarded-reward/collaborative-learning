import NodeStateMsg;
import SimulationMsg;


cplusplus(AdvertiseNeighbour::AdvertiseNeighbour){{
    this->setKind((int)SimulationMsgType::ADVERTISEMENT);
}}

message AdvertiseNeighbour extends SimulationMsg{
    NodeStateMsg state;
};
