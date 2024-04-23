#ifndef POWER_SOURCE_H
#define POWER_SOURCE_H

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
    float cost_per_mWh;

public:
    /**
     * Returns charge left in the power source.
    */
    virtual float getCharge() = 0;

    /**
     * Discharges the power source by the given amount.
     * Returns the actual amount discharged.
    */
    virtual float discharge(float amount) = 0;

    /**
     * Charges the power source by the given amount.
    */
    virtual void recharge(float amount){};

    void plug(){
        is_plugged = true;
    }

    void unplug(){
        is_plugged = false;
    }

    bool isPlugged(){
        return is_plugged;
    }

    void setCostPerMWh(float cost){
        cost_per_mWh = cost;
    }

    float getCostPerMWh(){
        return cost_per_mWh;
    }

};

#endif // POWER_SOURCE_H