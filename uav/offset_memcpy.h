#ifndef OFFSET_MEMCPY_H
#define OFFSET_MEMCPY_H

#include <stdio.h>

void memcpy_offset_source(void *destination, const void *source, size_t source_byte_offset, size_t size);
void memcpy_offset_dest(void *destination, const void *source, size_t dest_byte_offset, size_t size);

#endif //OFFSET_MEMCPY_H
