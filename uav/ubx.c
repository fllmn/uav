#include <string.h>
#include <stdio.h>
#include "sys_log.h"
#include "ubx.h"
#include "ubx-nav.h"
#include "ubx-cfg.h"

#define STATIC_HEADER_SIZE 6
#define CHECKSUM_SIZE 2

ubxFrame ubxStorage;
static const char sync_char[] = {0xB5, 0x62};

static uint16_t checksum(size_t length, uint8_t* buf)
{
    uint8_t checksumA = 0;
    uint8_t checksumB = 0;

    for(uint16_t i = 0; i < length; i++)
    {
        checksumA += *buf;
        checksumB += checksumA;
        buf++;
    }

    return checksumA << 8 | checksumB;
}
int calculateChecksum(ubxFrame *frame)
{
    int ret = -1;
    uint8_t *bytePointer = (uint8_t*) frame + sizeof(uint16_t);
    uint16_t chksm = checksum(frame->messageLength+4, bytePointer);

    memcpy(&frame->checksumA, &chksm, sizeof(uint16_t));
    ret = 0;
    return ret;
}

int validateChecksum(ubxFrame *frame)
{
    int ret = -1;
    uint8_t *bytePointer = (uint8_t*) frame + sizeof(uint16_t);
    uint8_t checksumA = 0;
    uint8_t checksumB = 0;
    uint16_t chksm = checksum(frame->messageLength+4, bytePointer);
    checksumA = (uint8_t) (chksm >> 8) & 0xFF;
    checksumB = (uint8_t) chksm & 0xFF;

    if (checksumA == frame->checksumA && checksumB == frame->checksumB)
    {
        ret = 0;
    }

    return ret;
}

int bufferToFrame(uint8_t *buffer)
{
    int ret = -1;

    memcpy(&ubxStorage, buffer, STATIC_HEADER_SIZE);
    memcpy(&ubxStorage.messagePayload, buffer + STATIC_HEADER_SIZE, ubxStorage.messageLength);
    memcpy(&ubxStorage.checksumA, buffer + STATIC_HEADER_SIZE + ubxStorage.messageLength, 2*sizeof(uint8_t));
    ret = ubxStorage.messageLength;
    return ret;
}

int frameToBuffer(uint8_t *buffer, size_t *length)
{
    int ret = -1;


    memcpy(&ubxStorage.syncChar, &sync_char, sizeof(sync_char));
    memcpy(buffer, &ubxStorage, ubxStorage.messageLength + STATIC_HEADER_SIZE);
    memcpy(buffer + ubxStorage.messageLength + STATIC_HEADER_SIZE, &ubxStorage.checksumA, 2*sizeof(uint8_t));
    *length = ubxStorage.messageLength + STATIC_HEADER_SIZE + CHECKSUM_SIZE;
    ret = 0;

    return ret;
}

void get_nav_enable_mess(uint8_t *buf, size_t *size)
{
    setMessageStat(&ubxStorage, NAV, NAV_PVT, 1);
    calculateChecksum(&ubxStorage);
    frameToBuffer((uint8_t*) buf, size);
}

double getLatitude(){
    positionType *latestPosition = getLatest();
    return latestPosition->latitude;
}

double getLongitude(){
    positionType *latestPosition = getLatest();
    return latestPosition->longitude;
}

void getLatestPosition(positionType *pos)
{
    memcpy(pos, getLatest(), sizeof(positionType));
}

messageClassType processMessage()
{
    messageClassType ret = UNKNOWN;

    switch(ubxStorage.messageClass)
    {
    case NAV:
        processNav(&ubxStorage);
        ret = NAV;
        break;
    case ACK:
        ret = ACK;
        break;
    }

    if (ret == UNKNOWN)
    {
        LOG_W("%s Unknown message type receives", __FUNCTION__);
    }

    return ubxStorage.messageClass;
}

void printBuf(uint8_t *buf)
{
    for (int i = 0; i < 70; i++)
    {
        printf("%02X", buf[i]);
    }
    printf("\n");
}

messageClassType process_buffer(uint8_t *buf, size_t *size)
{
    while(strncmp((char*)buf, sync_char, 2) != 0 && *size > 0)
    {
        buf++;
        (*size)--;
    }

    if (strncmp((char*)buf, sync_char, 2) != 0)
    {
        LOG_W("%s buffer did not contain and sync char", __FUNCTION__);
        return UNKNOWN;
    }

    size_t bytes_used = (size_t) bufferToFrame(buf) + 8;

    if (*size > bytes_used)
    {
        memmove(buf, &buf[bytes_used], ((*size)-bytes_used)*sizeof(uint8_t));
        memset(&buf[(*size)-bytes_used], 0, bytes_used*sizeof(uint8_t));
        *size = *size-bytes_used;
    }
    else
    {
        return UNKNOWN;
    }

    if (validateChecksum(&ubxStorage))
    {
        LOG_W("%s checksum failed", __FUNCTION__);
        return UNKNOWN;
    }

    return processMessage();
}



