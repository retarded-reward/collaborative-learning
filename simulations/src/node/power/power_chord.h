#ifndef POWER_CHORD_H
#define POWER_CHORD_H

#include "power_source.h"

class PowerChord : public PowerSource
{
protected:
    
    float discharge(float amount) override;
    float getCharge() override;
};

#endif // POWER_CHORD_H