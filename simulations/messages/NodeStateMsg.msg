cplusplus(h){{
#include "units.h" 
}}

class NodeStateMsg {
    float energy_percentage @cppType(percentage_t);
	float charge_rate_percentage @cppType(percentage_t);
	float queue_pop_percentage[] @cppType(percentage_t);	
};