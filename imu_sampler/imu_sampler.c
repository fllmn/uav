/**
 * @file rc_project_template.c
 *
 * This is meant to be a skeleton program for Robot Control projects. Change
 * this description and file name before modifying for your own purpose.
 */

#include <stdio.h>
#include <robotcontrol.h> // includes ALL Robot Control subsystems

// function declarations
void on_pause_press();
void on_pause_release();
int Initialize_Imu();
unsigned int buffer_idx= 0;
void dmp_callback();

#define 	MPU_BUFFER_SIZE   2000
// Global variables for IMU
rc_mpu_config_t mpu_config; 
struct mpu_buffer_element {
	double euler_angles[3];
	double accel[3];
	double gyro [3]; 	 
	double mag [3]; 	
	double temp;	
	uint64_t timestamp;
} ;
rc_mpu_data_t  	mpu_data;
struct mpu_buffer_element mpu_buffer[MPU_BUFFER_SIZE];

int main()
{
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


/* 	printf("\nPress and release pause button to turn green LED on and off\n");
	printf("hold pause button down for 2 seconds to exit\n"); */

	// Keep looping until state changes to EXITING
	rc_set_state(RUNNING);
	
	if (Initialize_Imu() == -1)
	{
		return -1;
	}
	
	while(rc_get_state()!=EXITING){
		// do things based on the state
		if(rc_get_state()==RUNNING){
			rc_led_set(RC_LED_GREEN, 1);
			rc_led_set(RC_LED_RED, 0);
		}
		else{
			rc_led_set(RC_LED_GREEN, 0);
			rc_led_set(RC_LED_RED, 1);
		}
		// always sleep at some point
		rc_usleep(100);
	}

	// turn off LEDs and close file descriptors
	rc_led_set(RC_LED_GREEN, 0);
	rc_led_set(RC_LED_RED, 0);
	rc_led_cleanup();
	rc_button_cleanup();	// stop button handlers
	rc_remove_pid_file();	// remove pid file LAST
	
	rc_mpu_power_off();
	return 0;
}


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
	printf("long press detected, shutting down\n");
	rc_set_state(EXITING);
	return;
}


int Initialize_Imu(){
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

void dmp_callback()
{
	memcpy(&mpu_buffer[buffer_idx].euler_angles, &mpu_data.dmp_TaitBryan, 	sizeof(double)*3 );
	memcpy(&mpu_buffer[buffer_idx].accel, &mpu_data.accel, 	sizeof(double)*3 );
	memcpy(&mpu_buffer[buffer_idx].gyro,  &mpu_data.gyro, 		sizeof(double)*3 );
	memcpy(&mpu_buffer[buffer_idx].mag,   &mpu_data.mag, 		sizeof(double)*3 );
	memcpy(&mpu_buffer[buffer_idx].temp,  &mpu_data.temp, 		sizeof(double) );
	mpu_buffer[buffer_idx].timestamp = rc_nanos_since_epoch();
	
	buffer_idx ++;
	printf("Bufffer idx %u \n", buffer_idx);
	if (buffer_idx > MPU_BUFFER_SIZE){
		printf("Bufffer wrap around\n");
	buffer_idx = 0;
	}
}





