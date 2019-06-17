#ifndef UBX_H
#define UBX_H

#include <stdint.h>
#include "ubx-types.h"


int calculateChecksum(ubxFrame* ubxStorage);
int validateChecksum(ubxFrame* ubxStorage);
void getLatestPosition(positionType *pos);
messageClassType process_buffer(uint8_t *buffer, size_t *size);
void get_nav_enable_mess(uint8_t *tx_buf, size_t *size);
double getLatitude();
double getLongitude();
#endif //UBX_H
