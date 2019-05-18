#ifndef GPS_H
#define GPS_H

typedef struct gps_pvt_t
{
	double latitude;
	double longitude;
	double altitude;
} gps_pvt_t;

int get_latest_pvt(gps_pvt_t *latestPvt);
void *gps_thread_func();

#endif //GPS_H
