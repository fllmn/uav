#include <robotcontrol.h>
int get_oldest_mpu_data(rc_mpu_data_t*);
int get_oldest_bmp_data(rc_bmp_data_t*);
void* i2c_thread_func();
