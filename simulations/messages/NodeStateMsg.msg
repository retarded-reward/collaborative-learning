enum NodePowerState{

	// node cannot receive and forward messages.
	// Also, the node is not advertising itself to other nodes.
	OFF = 0;
	// node can receive and forward messages,
	// but consumes power (even when idle).
	ON = 1;	
};

class NodeStateMsg {
    float energy;
	NodePowerState power_state;
	bool has_packet_in_buffer;
	int id;
	float data_cap; 
	NodeStateMsg *neighbour[] @owner;
};