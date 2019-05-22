#include "battery.h"
#include "circular_buffer.h"
#include <stdio.h>
#include <robotcontrol.h>
#define BATTERY_BUFFER_SIZE 10
#define BATTERY_SAMPLE_RATE 1000000

static FILE * battery_log;
static int battery_thread_ret_val;
cbuffer_handle_t battery_buffer;
bat_entry_t bat_entry;


static int init_battery_log()
{
    // add flag for enable and disable logging?
    battery_log = fopen("battery.log","w");

    if(battery_log == NULL)
    {
        printf("Could not open an Battery log file!\n");
        return -1;
    }
    fprintf(battery_log,"time voltage\n");
    return 0;
}

static int close_battery_log()
{
  // add flag for enable and disable logging?
  if(battery_log == NULL){
    return -1;
  }
  fclose(battery_log);
  return 0;
}

int initialize_battery_monitor()
{
  
  // initialize hardware first
  if(rc_adc_init()){
    fprintf(stderr,"ERROR: failed to run rc_init_adc()\n");
    return -1;
  }

  battery_buffer = create_cbuffer();
  int r1 =  cbuffer_init(battery_buffer,BATTERY_BUFFER_SIZE,sizeof(bat_entry_t));

  
  int r2 = init_battery_log();
  

  return r1||r2;
  
}

int finalize_battery_monitor()
{
  //does this type of return statement work for us 
  int r1 = rc_adc_cleanup();
  int r2 = cbuffer_free(battery_buffer);
  int r3 = close_battery_log();
  return r1||r2||r3;
}

int sample_battery_voltage(){
  bat_entry.time_ns = rc_nanos_since_epoch();//timestamp
  bat_entry.voltage = rc_adc_dc_jack();
  cbuffer_put(battery_buffer,&bat_entry);
  
  return 1;
}
  
int log_battery(){

    if(battery_log == NULL)
    {
        printf("Tried to write before battery log file was created\n");
        return -1;
    }
    bat_entry_t d;

    if(cbuffer_try_get(battery_buffer,&d))
    {
        //Log was empty
        return -1;
    }


    fprintf(battery_log,"%llu, %f;\n",
            d.time_ns,
            d.voltage
            );
    return 0;
}


int battery_main(){
  if ( initialize_battery_monitor()) return -1;
 
    while(rc_get_state()!=EXITING){
      rc_usleep(BATTERY_SAMPLE_RATE);
      sample_battery_voltage();
      log_battery();//dbg
    }
    if (finalize_battery_monitor()) return -1;
    return 0;
}


void* battery_thread_func() // wrapper function for the battery main which casts the retval to void*
{
   battery_thread_ret_val = battery_main();
    if (battery_thread_ret_val)
    {
        rc_set_state(EXITING); // ism this needed? OBS
    }
    return (void*)&battery_thread_ret_val;
}

