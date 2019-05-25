#ifndef AIRSPEED_H
#define AIRSPEED_H

#include <stdint.h>

typedef struct airspeed_t
{
    uint64_t time;
    double airspeed;
    double pressure;
    double temperature;
    uint32_t pressure_raw;
    uint32_t temperature_raw;
} airspeed_t;

int get_latest_airspeed(airspeed_t latest);
void log_airspeed(void);
void *airspeed_thread_func();

#endif //AIRSPEED_H
