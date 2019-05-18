#include "offset_memcpy.h"
#include <string.h>

void memcpy_offset_source(void *destination, const void *source, size_t source_byte_offset, size_t size) {
    char * source_offset = ((char*)source)+source_byte_offset;
    memcpy(destination, source_offset,size);
}

void memcpy_offset_dest(void *destination, const void *source, size_t dest_byte_offset, size_t size) {
    char * destination_offset = ((char*)destination)+dest_byte_offset;
    memcpy(destination_offset, source,size);
}
