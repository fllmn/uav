#include <stdint.h>
#include <string.h>
#include <math.h>
#include "ubx-types.h"
#include "ubx-nav.h"
#define MM2M 1000
#define DEGSCALING2RAD M_PI/180 * 0.0000001

positionType navPosition;

/*static double llh2EcefX(int32_t latitude, int32_t longitude, int32_t height)
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
  }*/

static int processEcef(ubxFrame *ubxStorage)
{
    int ret = -1;
    ecefFrame ecef;

    memcpy(&ecef, &ubxStorage->messagePayload, ubxStorage->messageLength);
    ret = -5;

    return ret;
}

static int processLlh(ubxFrame *ubxStorage)
{
    int ret = -1;
    llhFrame llh;

    memcpy(&llh, &ubxStorage->messagePayload, ubxStorage->messageLength);
    navPosition.latitude = llh.latitude * DEGSCALING2RAD;
    navPosition.longitude = llh.longitude * DEGSCALING2RAD;
    navPosition.altitude = llh.height * MM2M;
    navPosition.timeOfWeek = llh.timeOfWeek;
    ret = 0;

    return ret;
}

static int processPvt(ubxFrame *ubxStorage){
    int ret = -1;
    pvtFrame pvt;

    memcpy(&pvt, &ubxStorage->messagePayload, ubxStorage->messageLength);
    navPosition.latitude = pvt.latitude * DEGSCALING2RAD;
    navPosition.longitude = pvt.longitude * DEGSCALING2RAD;
    navPosition.altitude = pvt.height * MM2M;
    navPosition.timeOfWeek = pvt.timeOfWeek;

    ret = 0;

    return ret;
}

positionType* getLatest()
{
    return &navPosition;
}

int processNav(ubxFrame *ubxStorage)
{
    int ret = -1;

    switch (ubxStorage->messageId)
    {
        //case NAV_STATUS ... NAV_EOE:
        // Not impelemted
        //  break;
    case NAV_POSECEF:
        processEcef(ubxStorage);
        break;
    case NAV_POSLLH:
        processLlh(ubxStorage);
        break;
    case NAV_PVT:
        processPvt(ubxStorage);
        break;
        //default:
        // Raise error

    }
    return ret;
}

void getEcefPostion(positionType * latestPos)
{
    latestPos->latitude = navPosition.latitude;
    latestPos->longitude = navPosition.longitude;
    latestPos->altitude = navPosition.altitude;
    latestPos->timeOfWeek = navPosition.timeOfWeek;
}



