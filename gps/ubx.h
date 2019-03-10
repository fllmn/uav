#ifndef UBX_H
#define UBX_H

#define MAX_BYTES 256

typedef enum messageClassType __attribute__((__packed__))
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
};

typedef struct ubxFrame __attribute__((__packed__))
{
    uint16_t syncChar;
    messageClassType messageClass;
    messageIDType messageId;
    uint16_t messageLength;
    uint8_t messagePayload[MAX_BYTES];
    uint8_t checksumA;
    uint8_t checksumB;
};

int calculateChecksum(ubxFrame*);
int validateChecksum(ubxFrame*);
#endif //UBX_H
