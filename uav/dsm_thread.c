#include "dsm_thread.h"
#include <stdio.h>
#include <robotcontrol.h>
#include "circular_buffer.h"


static int dsm_thread_ret_val;
FILE * dsm_log;
#define	DSM_BUFFER_SIZE	  100
#define	NUM_CHANNELS	  8

// create a buffer and a data type which we will put on the buffer
cbuffer_handle_t dsm_buffer;
typedef struct dsm_entry{
  uint64_t time_ns;
  int channels[NUM_CHANNELS];
} dsm_entry_t;

dsm_entry_t dsm_entry;

int init_dsm_log()
{
	// add flag for enable and disable logging?
	dsm_log = fopen("dsm.log","w");

	if(dsm_log == NULL)
	{
		printf("Could not open an DSM log file!\n");   
		return -1;             
	}
	fprintf(dsm_log,"time ch1 ch2 ch3 ch4 ch5 ch6 ch7 ch8\n");
	return 0;
}

int close_dsm_log()
{
	// add flag for enable and disable logging?	
	fclose(dsm_log);
	return 0;
}
int log_dsm(){
	
	if(dsm_log == NULL)
	{
		printf("Tried to write before DSM log file was created\n");   
		return -1;             
	}
	dsm_entry_t d;
	
	if(cbuffer_try_get(dsm_buffer,&d))
	{
		//Log was empty
		return -1;
	}
   
   
	fprintf(dsm_log,"%llu, %d, %d, %d, %d, %d, %d, %d, %d;\n",
	d.time_ns,
	d.channels[0],
  d.channels[1],
  d.channels[2],
  d.channels[3],
  d.channels[4],
  d.channels[5],
  d.channels[6],
  d.channels[7]
	);
	return 0;	
}


// callback function which is called at every received packet 
void __send_pulses(void)
{
  int i, val;
  dsm_entry.time_ns = rc_nanos_since_epoch();
  // send single to working channels
  for(i=1; i<=NUM_CHANNELS; i++){
    val=rc_dsm_ch_raw(i);
    if(val>0){
      rc_servo_send_pulse_us(i,val);
      dsm_entry.channels[i-1]=val;
    }
  }
  // put data in the buffer
  cbuffer_put(dsm_buffer, &dsm_entry);
  return;
}
 
int dsm_main(){
	// Pre start of DSM Checks
  if(rc_dsm_init()==-1) return -1;
  if(rc_servo_init()==-1) return -1;
  if(rc_servo_power_rail_en(0)){
    fprintf(stderr,"ERROR: failed to disable power rail\n");
    return -1;
  }

  // Init dsm log
	// Init dsm buffer
	dsm_buffer = create_cbuffer();
	cbuffer_init(dsm_buffer,DSM_BUFFER_SIZE ,sizeof(dsm_entry_t));
	init_dsm_log();
  // set callback, we  are now listening and reacting to dsm packets
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
 

  //  clean up
  rc_dsm_cleanup();
  cbuffer_free(dsm_buffer);
  close_dsm_log();

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
