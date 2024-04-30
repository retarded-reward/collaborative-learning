#include "power_chord.h"
#include <limits>

using namespace std; 

mWh_t PowerChord::discharge(mWh_t amount){
    return amount;
}

float PowerChord::getCharge(){
    return numeric_limits<float>::max();
}