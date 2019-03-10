#include "ubx-nav.h"

#define MM2M 1000
#define DEGSCALING2RAD M_PI/180 * 0.00000001

positionType navPosition;

int processNav(ubxFrame *ubxStorage)
{
    int ret = -1;

    switch (ubxStorage->messageNavId)
    {
    case STATUS ... EOE:
        // Not impelemted
        break;
    case POSECEF:
        processEcef(ubxStorage);
        break;
    case POSLLH:
        processLlh(ubxStrorage);
        break;
    default:
        // Raise error
        
    }
    return ret;
}

void getEcefPostion(positionType * latestPos)
{
    *latestPos.X = navPosition.X;
    *latestPos.Y = navPosition.Y
    *latestPos.Z = navPosition.Z;
    *latestPos.TOW = navPosition.timeOfWeek;
}


static int processEcef(ubxFrame *ubxStorage)
{
    int ret = -1;
    ecefFrame ecef;

    memcpy(&ecef, &ubxStorage->messagePayload, ubxStorage->messageLength);
    navPosition.X = ((double) ecef.xCoordinate)/MM2M;
    navPosition.Y = ((double) ecef.yCoordinate)/MM2M;
    navPosition.Z = ((double) ecef.zCoordinate)/MM2M;
    navPosition.timeOfWeek = ecef.timeOfWeek;
    ret = 0;

    return ret;
}

static int processLlh(ubxFrame *ubxStorage)
{
    int ret = -1;
    llhFrame llh;

    memcpy(&llh, &ubxStorage->messagePayload, ubxStorage->messageLength);
    navPosition.X = llh2EcefX(llh.latitude, llh.longitude, llh.altitude);
    navPosition.Y = llh2EcefY(llh.latitude, llh.longitude, llh.altitude);
    navPosition.Z = llh2EcefZ(llh.latitude, llh.longitude, llh.altitude);
    navPosition.timeOfWeek = llh.timeOfWeek;
    ret = 0;

    return 0;
}


static double llh2EcefX(int32_t latitude, int32_t longitude, int32_t altitude)
{
    return 0;
}
static double llh2EcefY(int32_t latitude, int32_t longitude, int32_t altitude)
{
    return 0;
}
static double llh2EcefZ(int32_t latitude, int32_t longitude, int32_t altitude)
{
    return 0;
}
