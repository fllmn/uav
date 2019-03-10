#include "ubx-cfg.h"

static void setMessageHeader(ubxFrame *ubxStorage)
{
    ubxStorage->messageClass = messageClassType.CFG;
    ubxStorage->messageId = messageConfId.MSG;
}

int setMessageStat(ubxFrame *ubxStorage, uint8_t messageClass, uint8_t mesageId, uint8_t messageRate)
{
    int ret = -1;

    setMessageHeader(ubxStorage);

    ubxStorage->messageLength = 3;

    memcpy(ubxStorage.messagePayload, &messageClass, sizeof(uint8_t));
    memcpy(ubxStorage.messagePayload + 1, &messageId, sizeof(uint8_t));
    memcpy(ubxStorage.messagePayload + 2, &messageRate, sizeof(uint8_t));

    ret = 0;
    return ret;
    
}
