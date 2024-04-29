#include "battery.h"

#define sanitize_amount(amount) if (amount < 0) amount = - amount

Battery::Battery(float capacity){
    this->capacity = capacity;
    this->charge = capacity;
    this->cost_per_mWh = 0;
}

float Battery::getCharge(){
    return charge;
}

float Battery::discharge(float amount){
    
    float actual_amount;
    
    abort_if_unplugged(0);

    actual_amount = amount;
    
    sanitize_amount(amount);
    charge -= amount;
    if(charge < 0){
        actual_amount += charge;
        charge = 0;
    }

    return actual_amount;
}

void Battery::recharge(float amount){
    
    abort_if_unplugged();

    sanitize_amount(amount);
    charge += amount;
    if(charge > capacity){
        charge = capacity;
    }
}