#include "ubx.h"

#define STATIC_HEADER_SIZE 6
#define CHECKSUM_SIZE 2

ubxFrame ubxStorage

int calculateChecksum(ubxFrame *frame)
{
    int ret = -1;
    uint8_t *bytePointer = frame + sizeof(uint16_t);

    for(uint16_t i = 0; i < frame->messageLength+4;i++)
    {
        frame->checksumA = frame->checksumA + (uint8_t) *bytePointer;
        frame->checksumB = frame->checksumB + frame->checksumA;
        bytePointer++;
    }
    ret = 0;
    return ret;
}

int validateChecksum(ubxFrame *frame)
{
    int ret = -1;
    uint8_t *bytePointer = frame + sizeof(uint16_t);
    uint8_t checksumA = 0;
    uint8_t checksumB = 0;

    for(uint16_t i = 0; i < frame->messageLength+4;i++)
    {
        checksumA = checksumA + (uint8_t) *bytePointer;
        checksumB = checksumB + checksumA;
        bytePointer++;
    }

    if (checksumA == frame->checksumA && checksumB == frame->checksumB)
    {
        ret = 0;
    }

    return ret;
}

int populateStorage(uint8_t *buffer)
{
    int ret = -1;

    memcpy(&ubxStorage, buffer, STATIC_HEADER_SIZE);
    memcpy(&ubxStorage.messagePayload, buffer + STATIC_HEADER_SIZE, ubxStorage.messageLength + CHECKSUM_SIZE);
    ret = 0;
    return ret;
}

int processMessage()
{
    int ret = -1;

    switch(ubxStorage.messageID)
}
