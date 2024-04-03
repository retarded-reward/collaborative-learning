enum NodePowerState{

	OFF = 0;
	ON = 1;
};

class NodeStateMsg {
	
    float energy;
	NodePowerState power_state;
	bool has_packet_in_buffer;
	string id; 
	NodeStateMsg neighbour[];
};