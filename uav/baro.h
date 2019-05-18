#include <robotcontrol.h>

typedef struct bmp_entry {
    uint64_t time_ns;
    rc_bmp_data_t bmp_data;
} bmp_entry_t; // wraper for bmp type from rc_lib beacuse we need the timestamp

int get_oldest_bmp_data(rc_bmp_data_t*);
int initialize_baro();
int finalize_baro();
int get_latest_baro(bmp_entry_t *baro);
void sample_baro();
int log_baro();
