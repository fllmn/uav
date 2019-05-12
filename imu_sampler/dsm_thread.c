#include <robotcontrol.h>
static int dsm_thread_ret_val;
#define	DSM_BUFFER_SIZE	  100
unsigned int dsm_buffer_idx= 0;
int dsm_buffer[DSM_BUFFER_SIZE][8];

// callback function which is called at every received packet 
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
  if(rc_servo_power_rail_en(0)){
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
