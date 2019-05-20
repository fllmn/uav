#ifndef BATTERY_H
#define BATTERY_H
#include <stdint.h>
typedef struct bat_entry {
    uint64_t time_ns;
    double voltage;
} bat_entry_t; // wraper for bmp type from rc_lib beacuse we need the timestamp

int initialize_battery_monitor();
int finalize_battery_monitor();
int sample_battery_voltage();
int log_battery();
int get_oldest_battery_data();
int battery_main();
#endif //BATTERY_H
