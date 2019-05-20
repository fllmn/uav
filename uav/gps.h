#ifndef GPS_H
#define GPS_H

#include <stdint.h>

typedef struct gps_pvt_t
{
    uint16_t time_ns;
    double latitude;
    double longitude;
    double altitude;
} gps_pvt_t;

int get_latest_pvt(gps_pvt_t *latestPvt);
void log_gps(void);
void *gps_thread_func();

#endif //GPS_H
