#include "battery.h"

#define sanitize_amount(amount) if (amount < 0) amount = - amount

Battery::Battery(float capacity){
    this->capacity = capacity;
    this->charge = capacity;
}

float Battery::getCharge(){
    return charge;
}

float Battery::discharge(float amount){
    
    float actual_amount = amount;
    
    sanitize_amount(amount);
    charge -= amount;
    if(charge < 0){
        actual_amount += charge;
        charge = 0;
    }

    return actual_amount;
}

void Battery::recharge(float amount){
    sanitize_amount(amount);
    charge += amount;
    if(charge > capacity){
        charge = capacity;
    }
}

void Battery::deplete(){
    charge = 0;
}    
