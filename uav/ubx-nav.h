#ifndef UBX_NAV_H
#define UBX_NAV_H

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

typedef struct __attribute__((__packed__))
{
    uint32_t timeOfWeek;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    struct {
        uint8_t validDate 	: 1;
        uint8_t validTime 	: 1;
        uint8_t fullyResolved 	: 1;
        uint8_t validMag 	: 1;
        uint8_t reserved 	: 4;
    } valid;
    uint32_t timeAccuracyEst;
    int32_t fractionOfSec;
    uint8_t fixType;
    struct {
        uint8_t gnssFixOk	: 1;
        uint8_t diffSoln	: 1;
        uint8_t psmState	: 3;
        uint8_t headVehValid	: 1;
        uint8_t carrSoln	: 2;
    } flags;
    struct {
        uint8_t reserved	: 5;
        uint8_t confirmedAvai	: 1;
        uint8_t confirmedDate	: 1;
        uint8_t confirmedTime	: 1;
    } flags2;
    uint8_t numSV;
    int32_t longitude;
    int32_t latitude;
    int32_t height;
    int32_t hMSL;
    uint32_t horizontalAccuracy;
    uint32_t verticalAccuracy;
    int32_t velN;
    int32_t velE;
    int32_t velD;
    int32_t gSpeed;
    int32_t headingMotion;
    uint32_t speedAccuracy;
    uint32_t headingAccuracy;
    uint16_t positionDOP;
    uint8_t reserved[6];
    int32_t headingVehicle;
    int16_t magnDeclination;
    uint16_t magnDeclinationAccuracy;

} pvtFrame;

int processNav(ubxFrame *ubxStorage);
positionType* getLatest();
void getEcefPosition(positionType *latestPos);
#endif //UBX_NAV_H
