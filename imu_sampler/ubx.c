#include <string.h>
#include <stdio.h>
#include "ubx.h"
#include "ubx-nav.h"


#define STATIC_HEADER_SIZE 6
#define CHECKSUM_SIZE 2

ubxFrame ubxStorage;
static const char sync_char[] = {0xB5, 0x62};
int calculateChecksum(ubxFrame *frame)
{
    int ret = -1;
    uint8_t *bytePointer = (uint8_t*) frame + sizeof(uint16_t);

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
    uint8_t *bytePointer = (uint8_t*) frame + sizeof(uint16_t);
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

int bufferToFrame(uint8_t *buffer)
{
    int ret = -1;

    memcpy(&ubxStorage, buffer, STATIC_HEADER_SIZE);
    memcpy(&ubxStorage.messagePayload, buffer + STATIC_HEADER_SIZE, ubxStorage.messageLength);
    memcpy(&ubxStorage, buffer + STATIC_HEADER_SIZE + ubxStorage.messageLength, 2*sizeof(uint8_t));
    ret = ubxStorage.messagePayload;
    return ret;
}

int frameToBuffer(uint8_t *buffer, uint8_t *length)
{
    int ret = -1;

    memcpy(buffer, &ubxStorage, ubxStorage.messageLength + STATIC_HEADER_SIZE);
    memcpy(buffer + ubxStorage.messageLength + STATIC_HEADER_SIZE, &ubxStorage.checksumA, 2*sizeof(uint8_t));
    *length = ubxStorage.messageLength + STATIC_HEADER_SIZE + CHECKSUM_SIZE;
    ret = 0;

    return ret;
}

messageClassType processMessage()
{
    messageClassType ret = UNKNOWN; 

    switch(ubxStorage.messageId)
    {
    case NAV:
        processNav(&ubxStorage);
        ret = NAV;
        break;
    default:
        ret = UNKNOWN;
    }

    if (ret == UNKNOWN)
    {
        printf(stderr, "ERROR: %s Unknown message type receives", __FUNCTION__);
    }

    return ret;
}

messageClassType process_buffer(uint8_t *buf, size_t *size)
{
    while(strncmp(buf, sync_char, 2) != 0 && *size > 0)
    {
        buf++;
        (*size)--;
    }

    if (strncmp(buf, sync_char, 2) != 0)
    {
        printf(stderr, "ERROR: %s buffer did not contain and sync char", __FUNCTION__);
        return UNKNOWN;
    }

    int bytes_used = bufferToFrame(buf) + 4;

    if (*size > bytes_used)
    {
        memmove(buf, &buf[bytes_used], ((*size)-bytes_used)*sizeof(uint8_t));
        memset(&buf[*size], 0, (MAX_BYTES - (*size))*sizeof(uint8_t));
    } else
    {
        printf(stderr, "ERROR: %s buffer overflow", __FUNCTION__);
    }

    if (!validateChecksum(&ubxStorage))
    {
        printf(stderr, "ERROR: %s checksum failed", __FUNCTION__);
    }

    return processMessage(); 
}



