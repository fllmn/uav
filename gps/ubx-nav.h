#ifdef UBX_NAV_H
#define UBX_NAV_H

typedef enum messageNavId __attribute__((__packed__))
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
    VELNED = 0x12
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
};

typedef struct ecefFrame __attribute__((__packed__))
{
    uint32_t timeOfWeek;
    int32_t xCoordinate;
    int32_t yCoordinate;
    int32_t zCoordinate;
    uint32_t positionAccuracy;
};

typedef struct llhFrame __attribute__((__packed__))
{
    uint32_t timeOfWeek;
    int32_t longitude;
    int32_t latitude;
    int32_t height;
    int32_t heightMeanSeaLevel;
    uint32_t horzontalAccuracy;
    uint32_t verticalAccuracy;
};

typedef struct positionType
{
    double X;
    double Y;
    double Z;
    double timeOfWeek;
};

int processNav(ubxFrame *ubxStorage);
void getEcefPosition(positionType *latestPos)
#endif //UBX_NAV_H
