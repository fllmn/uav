#include <robotcontrol.h>
#include "i2c_thread.h"
#include "imu.h"
#include "baro.h"
// Global variables for IMU/DMP
static int i2c_thread_ret_val;

// need to keep functions which sample the i2c buss in the same thread 
int i2c_main(){

	if (initialize_baro()) return -1;
	if (initialize_imu() ) return -1;
	
	while(rc_get_state()!=EXITING){
		sample_imu();
		rc_usleep(100);
		sample_baro();
	}
	
	int r1 =finalize_imu();
	int r2 =finalize_baro();
	if (r1 || r2) return -1;
  
  
	return 0;
}

void* i2c_thread_func() // wrapper function for the i2c main which casts the retval to void*
{
	i2c_thread_ret_val = i2c_main();
	if (i2c_thread_ret_val)
    {
		rc_set_state(EXITING); // ism this needed? OBS
    }
	return (void*)&i2c_thread_ret_val;
}
 


