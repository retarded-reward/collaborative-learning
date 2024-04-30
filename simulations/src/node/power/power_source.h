#ifndef POWER_SOURCE_H
#define POWER_SOURCE_H

#include "units.h"

#define abort_if_unplugged(_ret) if(!is_plugged) return _ret

/**
 * A power source provides energy to actions that need it.
 * Each energy quantity is measured in mAh.
*/
class PowerSource{

protected:
    
    /**
     * When unplugged, the power source is not providing energy and charging it is 
     * uneffective.
    */
    bool is_plugged = false;
    reward_t cost_per_mWh;

public:
    /**
     * Returns charge left in the power source.
    */
    virtual mWh_t getCharge() = 0;

    /**
     * Return max charge storable in the Power Source
    */
    virtual mWh_t getCapacity() = 0;

    /**
     * Discharges the power source by the given amount.
     * Returns the actual amount discharged.
    */
    virtual mWh_t discharge(mWh_t amount) = 0;

    /**
     * Charges the power source by the given amount.
    */
    virtual void recharge(mWh_t amount){};

    void plug(){
        is_plugged = true;
    }

    void unplug(){
        is_plugged = false;
    }

    bool isPlugged(){
        return is_plugged;
    }

    void setCostPerMWh(reward_t cost){
        cost_per_mWh = cost;
    }

    reward_t getCostPerMWh(){
        return cost_per_mWh;
    }

};

#endif // POWER_SOURCE_H