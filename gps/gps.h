#ifndef GPS_H
#define GPS_H

struct gps_sol
{
    double latitude;
    double longitude;
    double altitude;
    double vel_n;
    double vel_e;
    double vel_d;
    long solValid;
}

int getLatestSolution(gps_sol*);

int getGpsTime(long * time);

#endif //GPS_H
