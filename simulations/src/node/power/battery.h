#ifndef BATTERY_H
#define BATTERY_H

#include "power_source.h"

class Battery : public PowerSource{
protected:
    mWh_t charge;
    mWh_t capacity; 

public:
    Battery(mWh_t capacity);

    mWh_t getCharge() override;

    mWh_t discharge(mWh_t amount) override;
        
    void recharge(mWh_t amount) override;
};

#endif // BATTERY_H