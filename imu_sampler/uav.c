#include <stdio.h>
#include <robotcontrol.h> // includes ALL Robot Control subsystems
#include "gps.h"
#include "i2c_thread.h"
#include "dsm_thread.h"

// threads
static pthread_t i2c_thread;
static pthread_t dsm_thread;
static pthread_t gps_thread;




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
    fprintf(stderr,"ERROR: Failed to start signal handler\n");
    return -1;
  }

  // initialize pause button
  if(rc_button_init(RC_BTN_PIN_PAUSE, RC_BTN_POLARITY_NORM_HIGH,
		    RC_BTN_DEBOUNCE_DEFAULT_US)){
    fprintf(stderr,"ERROR: Failed to initialize pause button\n");
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
  if(rc_pthread_create(&i2c_thread, i2c_thread_func, NULL, SCHED_OTHER, 0)){
    fprintf(stderr, "ERROR: Failed to start I2C sampler thread\n");
    return -1;
  } 
 
  if(rc_pthread_create(&dsm_thread, dsm_thread_func, NULL, SCHED_OTHER, 0)){
    fprintf(stderr, "ERROR: Failed to start DSM thread\n");
    return -1;
  } 

  if(rc_pthread_create(&gps_thread, gps_thread_func, NULL, SCHED_OTHER, 0)){
    fprintf(stderr, "ERROR: Failed to start GPS thread\n");
    return -1;
  } 
  //gps_main(1);
  // Sleep and let threads work
  while(rc_get_state()==RUNNING){
    sleep(1);

  }		
	
  // join i2c thread with 1.5s timeout
  ret = rc_pthread_timed_join(i2c_thread, &thread_retval, 1.5);
  if ( ret == 1){
    fprintf(stderr,"ERROR: IMU thread timed out\n");
  }
  printf("I2c thread returned:%d\n",*(int*)thread_retval);
  
 
  ret = rc_pthread_timed_join(dsm_thread, &thread_retval, 1.5);
  if ( ret == 1){
    fprintf(stderr,"ERROR: DSM thread timed out\n");
  }
  printf("DSM thread returned:%d\n",*(int*)thread_retval);
	

  ret = rc_pthread_timed_join(gps_thread, &thread_retval, 1.5);
  if ( ret == 1){
    fprintf(stderr,"ERROR: GPS thread timed out\n");
  }
  printf("GPS thread returned:%d\n",*(int*)thread_retval);

  // turn off LEDs and close file descriptors
  rc_led_set(RC_LED_GREEN, 0);
  rc_led_set(RC_LED_RED, 0);
  rc_led_cleanup();
  rc_button_cleanup();	// stop button handlers
  rc_remove_pid_file();	// remove pid file LAST
	
	

  return 0;
}


