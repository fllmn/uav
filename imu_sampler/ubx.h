#ifndef UBX_H
#define UBX_H

#include <stdint.h>
#include "ubx-types.h"

typedef enum __attribute__((__packed__)) messageClassType
{
    UNKNOWN = 0x00,
    NAV = 0x01,
    RXM = 0x02,
    INF = 0x04,
    ACK = 0x05,
    CFG = 0x06,
    UPD = 0x09,
    MON = 0x0A,
    AID = 0x0B,
    TIM = 0x0D,
    ESF = 0x10,
    MGA = 0x13,
    LOG = 0x21,
    SEC = 0x27,
    HNR = 0x28
} messageClassType;


int calculateChecksum(ubxFrame* ubxStorage);
int validateChecksum(ubxFrame* ubxStorage);
void getLatestPosition(positionType *pos);
messageClassType process_buffer(uint8_t *buffer, size_t *size);
void get_nav_enable_mess(uint8_t *tx_buf, size_t *size);
double getLatitude();
double getLongitude();
#endif //UBX_H
