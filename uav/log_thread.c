#include "log_thread.h"
#include <robotcontrol.h>
#include "imu.h"
#include "baro.h"
#include "dsm_thread.h"
#include "gps.h"

#define LOG_DELAY_US   1000000

static int log_thread_ret_val;


void* log_thread_func()
{
	rc_usleep(LOG_DELAY_US);
	while(rc_get_state()!=EXITING){
		
		rc_usleep(5000); 
		log_imu();
		log_baro();
		log_dsm();
		log_battery();
                log_gps();
                log_airspeed();
	}
	log_thread_ret_val = 0;
	return (void*)&log_thread_ret_val;
}

