#include "ubx-nav.h"

#define MM2M 1000
ecefFrame ecef;
llhFrame llh;

struct {
    double ecefXCoordinate;
    double ecefYCoordinate;
    double ecefZCoordinate;
    uint32_t timeOfWeek;
} navPosition;
int processNav(ubxFrame *ubxStorage)
{
    int ret = -1;

    switch (ubxStorage->messageNavId)
    {
    case POSECEF:
        processEcef(ubxStorage);
        break;
    }
    return ret;
}

int processEcef(ubxFrame *ubxStorage)
{
    int ret = -1;

    memcpy(&ecef, &ubxStorage->messagePayload, ubxStorage->messageLength);
    navPosition.ecefXCoordinate = ((double) ecef.xCoordinate)/MM2M;
    navPosition.ecefYCoordinate = ((double) ecef.yCoordinate)/MM2M;
    navPosition.ecefZCoordinate = ((double) ecef.zCoordinate)/MM2M;
    navPosition.timeOfWeek = ecef.timeOfWeek;
    ret = 0;

    return ret;
}

int processLlh(ubxFrame *ubxStorage)
{
    int ret = -1;

    memcpy(&llh, &ubxStorage->messagePayload, ubxStorage->messageLength);
    navPosition.ecefXCoordinate = llh2EcefX(llh.latitude, llh.longitude, llh.altitude);
    navPosition.ecefYCoordinate = llh2EcefY(llh.latitude, llh.longitude, llh.altitude);
    navPosition.ecefZCoordinate = llh2EcefZ(llh.latitude, llh.longitude, llh.altitude);
    ret = 0;

    return 0;
}
