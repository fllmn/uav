#ifndef UBX_CFG_H
#define UBX_CFG_H


typedef enum messageConfId __attribute__((__packed__))
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
}

#endif //UBX_CFG_H
