#ifndef POWER_MODEL_H_
#define POWER_MODEL_H_

#include <cstddef>

using namespace std;

class NICPowerModel{
protected:
    float tx_mW;
    float idle_mW;

public:
    //     
    float calc_tx_consumption_mWs(size_t bits, float rate_bps){
        return tx_mW * (bits / rate_bps);
    }

    float calc_idle_consumption_mWs(simtime_t idle_time) {
        return idle_mW * idle_time.dbl();
    }

    float getTx_mW() const {
        return tx_mW;
    }

    void setTx_mW(float value) {
        tx_mW = value;
    }

    float getIdle_mW() const {
        return idle_mW;
    }

    void setIdle_mW(float value) {
        idle_mW = value;
    }
};

#endif /* POWER_MODEL_H_ */