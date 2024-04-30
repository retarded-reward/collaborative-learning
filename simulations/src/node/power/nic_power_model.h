#ifndef POWER_MODEL_H_
#define POWER_MODEL_H_

#include <cstddef>
#include <omnetpp.h>
#include "units.h"

using namespace omnetpp;
using namespace std;

class NICPowerModel{
protected:
    mW_t tx_mW;
    mW_t idle_mW;

public:
    mWs_t calc_tx_consumption_mWs(b_t bits, Mbps_t rate_bps){
        return tx_mW * (bits / rate_bps);
    }

    mWs_t calc_idle_consumption_mWs(simtime_t idle_time) {
        return idle_mW * idle_time.dbl();
    }

    mW_t getTx_mW() const {
        return tx_mW;
    }

    void setTx_mW(mW_t value) {
        tx_mW = value;
    }

    mW_t getIdle_mW() const {
        return idle_mW;
    }

    void setIdle_mW(mW_t value) {
        idle_mW = value;
    }
};

#endif /* POWER_MODEL_H_ */