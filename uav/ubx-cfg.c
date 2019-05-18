#include <stdint.h>
#include <string.h>
#include "ubx-types.h"
#include "ubx-cfg.h"
typedef enum  __attribute__((__packed__)) messageConfId
{
    PRT = 0x00,
    MSG = 0x01,
    INF = 0x02,
    RST = 0x04,
    DAT = 0x06,
    RATE = 0x08,
    CFG = 0x09,
    RXM = 0x11,
    ANT = 0x13,
    SBAS = 0x16,
    NMEA = 0x17,
    USB = 0x1B,
    ODO = 0x1E,
    NAVX5 = 0x23,
    NAV5 = 0x24,
    TP5 = 0x31,
    RINV = 0x34,
    ITFM = 0x39,
    PM2 = 0x3B,
    TMODE2 = 0x3D,
    GNSS = 0x3E,
    LOGFILTER = 0x47,
    TXSLOT = 0x53,
    PWR = 0x57,
    HNR = 0x5C,
    ESRC = 0x60,
    DOSC = 0x61,
    SMGR = 0x62,
    GEOFENCE = 0x69,
    FIXSEED = 0x84,
    DYNSEED = 0x85,
    PMS = 0x86,
} messageConfId;
static void setMessageHeader(ubxFrame *ubxStorage)
{
    ubxStorage->messageClass = 0x06;
    ubxStorage->messageId = MSG;
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

    //memcpy(&ubxStorage->messagePayload, &messageClass, sizeof(uint8_t));
    //memcpy(&ubxStorage->messagePayload + 1, &messageId, sizeof(uint8_t));
    //memcpy(&ubxStorage->messagePayload + 2, &messageRate, sizeof(uint8_t));

    ret = 0;
    return ret;

}
