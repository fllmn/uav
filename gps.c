#include "gps.h"
#include "ubx.h"
#include "robotcontrol.h"

gps_sol latestSolution;
pthread_mutex_t mutexLock;

int initGps(pthread_mutex_t *lock, int rxPin, int txPin)
{
    rc_pinmux_set(rxPin, PINMUX_UART);
    rc_pinmux_set(txPin, PINMUX_UART);
    mutexLock = *lock;
}

int getLatestSolution(gps_sol * solution)
{
    *solution = &latestSolution;
}
