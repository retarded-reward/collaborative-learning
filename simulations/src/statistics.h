#ifndef STATISTICS_H
#define STATISTICS_H

#include <omnetpp.h>
#include <cmath>

using namespace omnetpp;
using namespace std;

// TODO: find a way to move this in .ini file
// Choose negative powers of 2 to improve perfomance
#define EWMA_ALPHA_DEFAULT 0.25

#define MAX_QUANTITY_NAME_LEN 32

#define swallow_if_nan(_value) if (isnan(_value)) return false

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

    virtual bool process(simtime_t& t, double& value, cObject *details)
    {
      swallow_if_nan(value);

      average = (1 - alpha) * average + (alpha * value);
      value = average;

      return true;
    }

  public:
    EWMAFilter()
    {
        alpha = EWMA_ALPHA_DEFAULT;
        average = 0;
    }
};

/**
 * Computes the time elapsed between the given time value and the last time value
 * used with this filter.
*/
class sinceLastFilter : public cNumericResultFilter
{
  protected:
    simtime_t last_time;

    
    virtual bool process(simtime_t& t, double& value, cObject *details)
    {
      simtime_t inter_time;

      inter_time = t - last_time;
      last_time = t;
      value = inter_time.dbl();

      EV_DEBUG << "value: " << value << "\n";

      return true;
    }

  public:
    sinceLastFilter()
    {
        last_time = 0;
    }
};

/**
 * Filter that outputs the sum of signal values divided by the measurement
 * interval (simtime).
 */
class SumPerSimtimeFilter : public cNumericResultFilter
{
    protected:
        double sum;
    protected:
        virtual bool process(simtime_t& t, double& value, cObject *details)
        {
          sum += value;
          value = sum / simTime().dbl();
          return true;
        }
    public:
        SumPerSimtimeFilter() {sum = 0;}
};

Register_ResultFilter("sumPerDuration", SumPerSimtimeFilter);


Register_ResultFilter("ewma", EWMAFilter);
Register_ResultFilter("sinceLast", sinceLastFilter);
Register_ResultFilter("sumPerSimtime", SumPerSimtimeFilter);

/**
 * Measures a named quantity needed for a statistic computation.
 * 
 * Must be called from a cComponent object.
*/
#define measure_quantity(_name, _value) emit(registerSignal(_name), _value)

#define register_statistic_template(_quantity_name, _template_name)\
{\
  getEnvir()->addResultRecorders(this, registerSignal(_quantity_name), _quantity_name, getProperties()->get("statisticTemplate", _template_name));\
} 
#define init_statistic_template(_quantity_name_buffer, _statistic_template_name, _quantity_name_format, ...)\
{\
    snprintf(_quantity_name_buffer, MAX_QUANTITY_NAME_LEN, _quantity_name_format, __VA_ARGS__);\
    register_statistic_template(_quantity_name_buffer, _statistic_template_name);\
}


#endif // STATISTICS_H