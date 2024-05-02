#ifndef UNITS_H_INCLUDE
#define UNITS_H_INCLUDE

#include <cstddef>

typedef float percentage_t;
#define calc_percentage(_part, _full) (_full != 0)? _part * 100/ _full : 0  

// MilliWatt per Hour
typedef double mWh_t;
// MilliWatt per Second
typedef float mWs_t;
// MilliWatt
typedef float mW_t;

// Seconds. Note that simtime_t uses seconds.
typedef float s_t;

// millisecond
typedef float ms_t;

// MegaBit per Second
typedef float Mbps_t;

// bits
typedef size_t b_t;
// bytes
typedef size_t B_t;

// Reward unit type
typedef float reward_t;

#endif // UNITS_H_INCLUDE