#ifndef GPS_H
#define GPS_H

typedef struct
{
    double latitude;
    double longitude;
    double altitude;
    double vel_n;
    double vel_e;
    double vel_d;
    long solValid;
} gps_sol;

int getLatestSolution(gps_sol* gps_pos);

int getGpsTime(long * time);

#endif //GPS_H
