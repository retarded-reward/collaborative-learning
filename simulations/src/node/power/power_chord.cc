#include "power_chord.h"
#include <limits>

using namespace std; 

mWh_t PowerChord::discharge(mWh_t amount){
    return amount;
}

mWh_t PowerChord::getCharge(){

    abort_if_unplugged(0);
    return numeric_limits<float>::max();
}

mWh_t PowerChord::getCapacity()
{
    abort_if_unplugged(0);
    return numeric_limits<float>::max();
}
