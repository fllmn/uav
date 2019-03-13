#include "ubx-nav.h"
#include <stdint.h>
#include <string.h>
#include "ubx-types.h"
#define MM2M 1000
#define DEGSCALING2RAD M_PI/180 * 0.00000001

typedef enum  __attribute__((__packed__)) messageNavId
{
    UNKNOWN = 0x00,
    POSECEF = 0x01,
    POSLLH = 0x02,
    STATUS = 0x03,
    DOP = 0x04,
    ATT = 0x05,
    SOL = 0x06,
    PVT = 0x07,
    ODO = 0x09,
    RESETODO = 0x10,
    VELECEF = 0x11,
    VELNED = 0x12,
    TIMEGPS = 0x20,
    TIMEUTC = 0x21,
    CLOCK = 0x22,
    TIMEBDS = 0x24,
    TIMEGAL = 0x25,
    TIMELS = 0x26,
    SVINFO = 0x30,
    DGPS = 0x31,
    SBAS = 0x32,
    ORB = 0x34,
    SAT = 0x35,
    GEOFENCE = 0x39,
    AOPSTATUS = 0x60,
    EOE = 0x61,
} messageNavId;

positionType navPosition;

static double llh2EcefX(int32_t latitude, int32_t longitude, int32_t height)
{
    return (double) latitude + longitude + height;
}
static double llh2EcefY(int32_t latitude, int32_t longitude, int32_t height)
{
    return (double) latitude + longitude + height;
}
static double llh2EcefZ(int32_t latitude, int32_t longitude, int32_t height)
{
    return (double) latitude + longitude + height;
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
    navPosition.X = llh2EcefX(llh.latitude, llh.longitude, llh.height);
    navPosition.Y = llh2EcefY(llh.latitude, llh.longitude, llh.height);
    navPosition.Z = llh2EcefZ(llh.latitude, llh.longitude, llh.height);
    navPosition.timeOfWeek = llh.timeOfWeek;
    ret = 0;

    return ret;
}

int processNav(ubxFrame *ubxStorage)
{
    int ret = -1;

    switch (ubxStorage->messageId)
    {
    case STATUS ... EOE:
        // Not impelemted
        break;
    case POSECEF:
        processEcef(ubxStorage);
        break;
    case POSLLH:
        processLlh(ubxStorage);
        break;
    //default:
        // Raise error
        
    }
    return ret;
}

void getEcefPostion(positionType * latestPos)
{
    latestPos->X = navPosition.X;
    latestPos->Y = navPosition.Y;
    latestPos->Z = navPosition.Z;
    latestPos->timeOfWeek = navPosition.timeOfWeek;
}



