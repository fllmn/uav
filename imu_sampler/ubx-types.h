
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

typedef struct  __attribute__((__packed__))
{
    uint32_t timeOfWeek;
    int32_t xCoordinate;
    int32_t yCoordinate;
    int32_t zCoordinate;
    uint32_t positionAccuracy;
} ecefFrame;

typedef struct  __attribute__((__packed__))
{
    uint32_t timeOfWeek;
    int32_t longitude;
    int32_t latitude;
    int32_t height;
    int32_t heightMeanSeaLevel;
    uint32_t horzontalAccuracy;
    uint32_t verticalAccuracy;
} llhFrame;

typedef struct positionType
{
    double X;
    double Y;
    double Z;
    double timeOfWeek;
} positionType;
