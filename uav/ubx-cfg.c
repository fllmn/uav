#include <stdint.h>
#include <string.h>
#include "ubx-types.h"
#include "ubx-cfg.h"

typedef enum  __attribute__((__packed__)) messageConfId
{
   CFG_PRT = 0x00,
   CFG_MSG = 0x01,
   CFG_INF = 0x02,
   CFG_RST = 0x04,
   CFG_DAT = 0x06,
   CFG_RATE = 0x08,
   CFG_CFG = 0x09,
   CFG_RXM = 0x11,
   CFG_ANT = 0x13,
   CFG_SBAS = 0x16,
   CFG_NMEA = 0x17,
   CFG_USB = 0x1B,
   CFG_ODO = 0x1E,
   CFG_NAVX5 = 0x23,
   CFG_NAV5 = 0x24,
   CFG_TP5 = 0x31,
   CFG_RINV = 0x34,
   CFG_ITFM = 0x39,
   CFG_PM2 = 0x3B,
   CFG_TMODE2 = 0x3D,
   CFG_GNSS = 0x3E,
   CFG_LOGFILTER = 0x47,
   CFG_TXSLOT = 0x53,
   CFG_PWR = 0x57,
   CFG_HNR = 0x5C,
   CFG_ESRC = 0x60,
   CFG_DOSC = 0x61,
   CFG_SMGR = 0x62,
   CFG_GEOFENCE = 0x69,
   CFG_FIXSEED = 0x84,
   CFG_DYNSEED = 0x85,
   CFG_PMS = 0x86,
} messageConfId;

static void setMessageHeader(ubxFrame *ubxStorage)
{
    ubxStorage->messageClass = CFG;
    ubxStorage->messageId = CFG_MSG;
}

int setMessageStat(ubxFrame *ubxStorage, uint8_t messageClass, uint8_t messageId, uint8_t messageRate)
{
    int ret = -1;

    setMessageHeader(ubxStorage);

    ubxStorage->messageLength = 3;

    ubxStorage->messagePayload[0] = messageClass;
    ubxStorage->messagePayload[1] = messageId;
    ubxStorage->messagePayload[2] = messageRate;
    ubxStorage->messageLength = 3;

    ret = 0;
    return ret;

}
