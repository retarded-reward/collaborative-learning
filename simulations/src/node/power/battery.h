#ifndef BATTERY_H
#define BATTERY_H

#include "power_source.h"

class Battery : public PowerSource{
protected:
    float charge;
    float capacity;

public:
    Battery(float capacity);

    float getCharge() override;

    float discharge(float amount) override;
        
    void recharge(float amount) override;

    void deplete() override;
};

#endif // BATTERY_H