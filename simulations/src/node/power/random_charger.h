#ifndef RANDOM_CHARGER_H_
#define RANDOM_CHARGER_H_

#include "power_source.h"
#include <omnetpp.h>

using namespace omnetpp;

class RandomCharger : public PowerSource{
protected:
    cPar &distribution;
    mWh_t charge_cap;

public:
    RandomCharger(cPar &distribution, mWh_t charge_cap): distribution(distribution){
        this->charge_cap = charge_cap;
    }

    /**
     * The user provided amount is interpreted as the desired amount of
     * energy. The actual amount is computed using the provided
     * distribution.
    */
    mWh_t discharge(mWh_t amount) override {
        
        mWh_t actual_amount;
        double random;
        
        abort_if_unplugged(0);

        random = distribution.doubleValue();
        random = absolute(random);
        actual_amount = amount * random;
        return actual_amount;
    }

    /**
     * Returns the charge capacity of the charger.
    */
    mWh_t getCharge() override {
        abort_if_unplugged(0);
        return getCapacity();
    }

    mWh_t getCapacity() override {
        return charge_cap;
    }
};

#endif /* RANDOM_CHARGER_H_ */