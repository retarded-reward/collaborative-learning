#include "power_chord.h"
#include <limits>

using namespace std; 

float PowerChord::discharge(float amount){
    return amount;
}

float PowerChord::getCharge(){
    return numeric_limits<float>::max();
}