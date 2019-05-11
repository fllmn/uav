#include <stdio.h>
#include <robotcontrol.h> // includes ALL Robot Control subsystems

// function declarations
/* void on_pause_press();
void on_pause_release();
int initialize_imu();
int bmp_main();
void dmp_callback(); */



// Global variables for IMU/DMP
static pthread_t i2c_thread;
static int i2c_thread_ret_val;
#define 	MPU_BUFFER_SIZE   200
rc_mpu_config_t mpu_config; 
struct mpu_buffer_element {
	double euler_angles[3];
	double accel[3];
	double gyro [3]; 	 
	double mag [3]; 	
	double temp;	
	uint64_t timestamp;
};
rc_mpu_data_t  	mpu_data; // Allocate a Motion processing data structure
unsigned int mpu_buffer_idx= 0;
struct mpu_buffer_element mpu_buffer[MPU_BUFFER_SIZE];


//global vars for bmp
#define		BMP_BUFFER_SIZE	  100
#define 	BMP_CHECK_HZ    25
rc_bmp_data_t   baro_data; // Allocate a Barometer struct
struct bmp_buffer_element {
	double temp_c;
	double alt_m;
	double pressure_pa;	 
	uint64_t timestamp;
};
struct bmp_buffer_element bmp_buffer[BMP_BUFFER_SIZE];
unsigned int bmp_buffer_idx= 0;

// vars for DSM
static pthread_t dsm_thread;
static int dsm_thread_ret_val;
#define	DSM_BUFFER_SIZE	  100
unsigned int dsm_buffer_idx= 0;
int dsm_buffer[DSM_BUFFER_SIZE][8];



/**
 * Make the Pause button toggle between paused and running states.
 */
void on_pause_release()
{
	if(rc_get_state()==RUNNING)	rc_set_state(PAUSED);
	else if(rc_get_state()==PAUSED)	rc_set_state(RUNNING);
	return;
}

/**
* If the user holds the pause button for 2 seconds, set state to EXITING which
* triggers the rest of the program to exit cleanly.
**/
void on_pause_press()
{
	int i;
	const int samples = 100; // check for release 100 times in this period
	const int us_wait = 2000000; // 2 seconds

	// now keep checking to see if the button is still held down
	for(i=0;i<samples;i++){
		rc_usleep(us_wait/samples);
		if(rc_button_get_state(RC_BTN_PIN_PAUSE)==RC_BTN_STATE_RELEASED) return;
	}
	printf("Long press detected, shutting down\n");
	rc_set_state(EXITING);
	return;
}

// IMU/DMP functions
void dmp_callback()
{
	memcpy(&mpu_buffer[mpu_buffer_idx].euler_angles, &mpu_data.dmp_TaitBryan, 	sizeof(double)*3 );
	memcpy(&mpu_buffer[mpu_buffer_idx].accel, &mpu_data.accel, 	sizeof(double)*3 );
	memcpy(&mpu_buffer[mpu_buffer_idx].gyro,  &mpu_data.gyro, 		sizeof(double)*3 );
	memcpy(&mpu_buffer[mpu_buffer_idx].mag,   &mpu_data.mag, 		sizeof(double)*3 );
	memcpy(&mpu_buffer[mpu_buffer_idx].temp,  &mpu_data.temp, 		sizeof(double) );
	mpu_buffer[mpu_buffer_idx].timestamp = rc_nanos_since_epoch();
	
	printf("MPU write at bufffer idx %u \n", mpu_buffer_idx);
	mpu_buffer_idx ++;
	
	if (mpu_buffer_idx > MPU_BUFFER_SIZE){
		//printf("MPU bufffer wrap around\n");		
		mpu_buffer_idx = 0;
	}
}

int initialize_imu(){
	// Get default config
	if (rc_mpu_set_config_to_default(&mpu_config) == -1)
	{
		fprintf(stderr,"ERROR: failed to reset dmp config\n");
		return -1;
	}
	
	// Modify config
	mpu_config.enable_magnetometer = 1;
	mpu_config.dmp_fetch_accel_gyro = 1;
	mpu_config.dmp_sample_rate = 200;
	
	// initialize with config and pointer to data structure
	if (rc_mpu_initialize_dmp(&mpu_data,mpu_config) == -1)
	{
		fprintf(stderr,"ERROR: failed to initialize mpu in dmp mode\n");
		return -1;
	}

	// Wet which function should be called when data is ready
	if (rc_mpu_set_dmp_callback	(dmp_callback) == -1)
	{
		fprintf(stderr,"ERROR: failed to initialize dmp callback\n");
		return -1;
	}
	

	return 1;
	
}


// need to keep functions which sample the i2c buss in the same thread 
int i2c_main(){
	// initialize dmp/imu interupt
	if (initialize_imu() == -1) {return -1;}
	
	if(rc_bmp_init(BMP_OVERSAMPLE_16, BMP_FILTER_OFF))    return -1;
	if(rc_bmp_read(&baro_data)) {return -1;}
	
	
	while(rc_get_state()!=EXITING){
        rc_usleep(1000000/BMP_CHECK_HZ);
        // perform the i2c reads to the sensor, on bad read just try later
        if(rc_bmp_read(&baro_data)) continue;
		
		bmp_buffer[bmp_buffer_idx].timestamp = rc_nanos_since_epoch();
		bmp_buffer[bmp_buffer_idx].temp_c=baro_data.temp_c;
		bmp_buffer[bmp_buffer_idx].alt_m = baro_data.alt_m;
		bmp_buffer[bmp_buffer_idx].pressure_pa = baro_data.pressure_pa;
		
		//printf("BMP write at bufffer idx %u \n", bmp_buffer_idx);
		bmp_buffer_idx ++;
		
		
		if (bmp_buffer_idx > BMP_BUFFER_SIZE){
			//printf("BMP bufffer wrap around\n");
			bmp_buffer_idx = 0;
		}
	
	}
	rc_mpu_power_off();
	rc_bmp_power_off();
	return 0;
}

void* i2c_thread_func() // wrapper function for the i2c main which casts the retval to void*
{
	i2c_thread_ret_val = i2c_main();
	if (i2c_thread_ret_val ==-1)
	{
		rc_set_state(EXITING);
	}
	 return (void*)&i2c_thread_ret_val;
}
 
 
void __send_pulses(void)
{
        int i, val;
        // send single to working channels
        for(i=1; i<=8; i++){
            val=rc_dsm_ch_raw(i);
            //if(val>0){
				rc_servo_send_pulse_us(i,val);
			//}
			// write to log
			dsm_buffer[dsm_buffer_idx][i] = val;
			printf("Val %d = %d ",i,val);
		
		}
		printf("\n");
		dsm_buffer_idx ++;
		if (dsm_buffer_idx > DSM_BUFFER_SIZE){
			//printf("DSM bufffer wrap around\n");		
			dsm_buffer_idx = 0;
		}  
    return;
}
 
int dsm_main(){
	if(rc_dsm_init()==-1) return -1;
	if(rc_servo_init()==-1) return -1;
	
	if(rc_adc_init()){
        fprintf(stderr,"ERROR: failed to run rc_adc_init()\n");
        return -1;
    }
    if(rc_adc_batt()<6.0){
        fprintf(stderr,"ERROR: battery disconnected or insufficiently charged to drive motors\n");
        return -1;
    }
    rc_adc_cleanup();
    if(rc_servo_power_rail_en(1)){
        fprintf(stderr,"ERROR: failed to enable power rail\n");
        return -1;
    }

	rc_dsm_set_callback(&__send_pulses);
	
	while(rc_get_state()!=EXITING){

		if(rc_dsm_is_connection_active()==0){
			printf("\rSeconds since last DSM packet: ");
			printf("%0.1f ", rc_dsm_nanos_since_last_packet()/1000000000.0);
			printf("                             ");
			fflush(stdout);
		}
	rc_usleep(2500);

	}	

	return 0;
}

void *dsm_thread_func(){
	dsm_thread_ret_val = dsm_main();
	if (dsm_thread_ret_val ==-1)
	{
		rc_set_state(EXITING);
	}
	 return (void*)&dsm_thread_ret_val;
}

int main()
{
	int ret;
	void* thread_retval; // return value of the thread
	// make sure another instance isn't running
	// if return value is -3 then a background process is running with
	// higher privaledges and we couldn't kill it, in which case we should
	// not continue or there may be hardware conflicts. If it returned -4
	// then there was an invalid argument that needs to be fixed.
	if(rc_kill_existing_process(2.0)<-2) return -1;

	// start signal handler so we can exit cleanly
	if(rc_enable_signal_handler()==-1){
		fprintf(stderr,"ERROR: failed to start signal handler\n");
		return -1;
	}

	// initialize pause button
	if(rc_button_init(RC_BTN_PIN_PAUSE, RC_BTN_POLARITY_NORM_HIGH,
						RC_BTN_DEBOUNCE_DEFAULT_US)){
		fprintf(stderr,"ERROR: failed to initialize pause button\n");
		return -1;
	}
	// Assign functions to be called when button events occur
	rc_button_set_callbacks(RC_BTN_PIN_PAUSE,on_pause_press,on_pause_release);

	// make PID file to indicate your project is running
	// due to the check made on the call to rc_kill_existing_process() above
	// we can be fairly confident there is no PID file already and we can
	// make our own safely.
	rc_make_pid_file();

	// Keep looping until state changes to EXITING
	rc_set_state(RUNNING);
	
	//start threads
/* 	if(rc_pthread_create(&i2c_thread, i2c_thread_func, NULL, SCHED_OTHER, 0)){
        fprintf(stderr, "ERROR: Failed to start I2C sampler thread\n");
        return -1;
    } */
	dsm_main();
/* 	if(rc_pthread_create(&dsm_thread, dsm_thread_func, NULL, SCHED_OTHER, 0)){
        fprintf(stderr, "ERROR: Failed to start DSM passthrough thread\n");
        return -1;
    } */
	
	
	// Sleep and let threads work
	while(rc_get_state()==RUNNING){
		sleep(1);

	}		
	
    // join i2c thread with 1.5s timeout
/* 	ret = rc_pthread_timed_join(i2c_thread, &thread_retval, 1.5);
	if ( ret == 1){
		fprintf(stderr,"ERROR: IMU thread timed out\n");
	}
	printf("I2c thread  returned:%d\n",*(int*)thread_retval);
 */
/* 	ret = rc_pthread_timed_join(dsm_thread, &thread_retval, 1.5);
	if ( ret == 1){
		fprintf(stderr,"ERROR: DSM passthrough thread timed out\n");
	}
	printf(" DSM passthrough thread  returned:%d\n",*(int*)thread_retval);
	
	 */
	// turn off LEDs and close file descriptors
	rc_led_set(RC_LED_GREEN, 0);
	rc_led_set(RC_LED_RED, 0);
	rc_led_cleanup();
	rc_button_cleanup();	// stop button handlers
	rc_remove_pid_file();	// remove pid file LAST
	
	

	return 0;
}


