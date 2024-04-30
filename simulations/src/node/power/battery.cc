#include "battery.h"

#define sanitize_amount(amount) if (amount < 0) amount = - amount

Battery::Battery(mWh_t capacity){
    this->capacity = capacity;
    this->charge = capacity;
    this->cost_per_mWh = 0;
}

mWh_t Battery::getCharge(){
    return charge;
}

mWh_t Battery::discharge(mWh_t amount){
    
    mWh_t actual_amount;
    
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

void Battery::recharge(mWh_t amount){
    
    abort_if_unplugged();

    sanitize_amount(amount);
    charge += amount;
    if(charge > capacity){
        charge = capacity;
    }
}