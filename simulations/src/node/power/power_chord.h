#ifndef POWER_CHORD_H
#define POWER_CHORD_H

#include "power_source.h"

class PowerChord : public PowerSource
{
protected:
    
    mWh_t discharge(mWh_t amount) override;
    mWh_t getCharge() override;
};

#endif // POWER_CHORD_H