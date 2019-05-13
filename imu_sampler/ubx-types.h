
#define MAX_BYTES 256


typedef struct __attribute__((__packed__)) ubxFrame
{
    uint16_t syncChar;
    uint8_t messageClass;
    uint8_t messageId;
    uint16_t messageLength;
    uint8_t messagePayload[MAX_BYTES];
    uint8_t checksumA;
    uint8_t checksumB;
} ubxFrame;


typedef struct positionType
{
    double latitude;
    double longitude;
    double altitude;
    double timeOfWeek;
} positionType;

typedef enum  __attribute__((__packed__)) messageNavId
{
    NAV_UNKNOWN = 0x00,
    NAV_POSECEF = 0x01,
    NAV_POSLLH = 0x02,
    NAV_STATUS = 0x03,
    NAV_DOP = 0x04,
    NAV_ATT = 0x05,
    NAV_SOL = 0x06,
    NAV_PVT = 0x07,
    NAV_ODO = 0x09,
    NAV_RESETODO = 0x10,
    NAV_VELECEF = 0x11,
    NAV_VELNED = 0x12,
    NAV_TIMEGPS = 0x20,
    NAV_TIMEUTC = 0x21,
    NAV_CLOCK = 0x22,
    NAV_TIMEBDS = 0x24,
    NAV_TIMEGAL = 0x25,
    NAV_TIMELS = 0x26,
    NAV_SVINFO = 0x30,
    NAV_DGPS = 0x31,
    NAV_SBAS = 0x32,
    NAV_ORB = 0x34,
    NAV_SAT = 0x35,
    NAV_GEOFENCE = 0x39,
    NAV_AOPSTATUS = 0x60,
    NAV_EOE = 0x61,
} messageNavId;

