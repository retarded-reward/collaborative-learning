#ifndef POWER_SOURCE_H
#define POWER_SOURCE_H

class PowerSource{
public:
    /**
     * Returns percentage of charge left in the power source.
    */
    virtual float getCharge() = 0;

    /**
     * Discharges the power source by the given amount.
     * Returns the actual amount discharged.
    */
    virtual float discharge(float amount){};

    /**
     * Charges the power source by the given amount.
    */
    virtual void recharge(float amount){};

    /**
     * Depletes the power source.
     * The discharge() method always returns 0 until the power source is
     * recharged.
    */
    virtual void deplete(){};

};

#endif // POWER_SOURCE_H