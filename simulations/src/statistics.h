#ifndef STATISTICS_H
#define STATISTICS_H

#include <omnetpp.h>
#include <cmath>

using namespace omnetpp;
using namespace std;

// TODO: find a way to move this in .ini file
// Choose negative powers of 2 to improve perfomance
#define EWMA_ALPHA_DEFAULT 0.25

/**
 * The Exponential Weighted Moving Average is a special kind of
 * average that gives more weight to recent values
 * according to the alpha parameter.
 * https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average 
*/
class EWMAFilter : public cNumericResultFilter
{
  protected:
    double alpha;
    double average;

    virtual bool process(simtime_t& t, double& value, cObject *details);

  public:
    EWMAFilter()
    {
        alpha = EWMA_ALPHA_DEFAULT;
        average = 0;
    }
  
};

bool EWMAFilter::process(simtime_t& t, double& value, cObject *details)
{
    if (isnan(value)) return false;

    average = (1 - alpha) * average + (alpha * value);
    value = average;

    return true;
}

Register_ResultFilter("ewma", EWMAFilter);

/**
 * Measures a named quantity needed for a statistic computation.
 * 
 * Must be called from a cComponent object.
*/
#define measure_quantity(_name, _value) emit(registerSignal(_name), _value)

/**
 * Measures a named time span needed for a statistic computation.
 * 
 * Must be called from a cComponent object.
*/
#define measure_time(_name, _code)\
do {\
    simtime_t _start;\
    simtime_t _elapsed;\
    \
    _start = simTime();\
    _code;\
    _elapsed = simTime() - _start;\
    measure_quantity(_name, _elapsed);\
} while (0)

#endif // STATISTICS_H