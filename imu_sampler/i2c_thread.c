#include "i2c_thread.h"
#include "circular_buffer.h"
// Global variables for IMU/DMP
static int i2c_thread_ret_val;

#define MPU_BUFFER_SIZE   10
rc_mpu_config_t mpu_config; 
rc_mpu_data_t  	mpu_data; // Declare a Motion processing data structure
cbuffer_handle_t mpu_buffer; //Declare a cbuffer


//global vars for bmp (barometer)
#define		BMP_BUFFER_SIZE	  10
#define 	BMP_CHECK_HZ    25
rc_bmp_data_t   bmp_data; // Declare a Barometer struct
cbuffer_handle_t bmp_buffer; //Declare a cbuffer

// IMU/DMP functions
void dmp_callback()
{
  // When this callback is called we know a new set of mpu_data has been sampled
  cbuffer_put(mpu_buffer,&mpu_data);
	
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
	
	
  return 0;
	
}


// need to keep functions which sample the i2c buss in the same thread 
int i2c_main(){
  // initialize dmp/imu interupt

	
  if(rc_bmp_init(BMP_OVERSAMPLE_16, BMP_FILTER_OFF))    return -1;
  if(rc_bmp_read(&bmp_data)) {return -1;}
  
  // Init mpu buffer
  mpu_buffer = empty_cbuffer();
  cbuffer_init(mpu_buffer,MPU_BUFFER_SIZE ,sizeof(rc_mpu_data_t) );
	
  // Init bmp buffer
  bmp_buffer = empty_cbuffer();
  cbuffer_init(bmp_buffer,BMP_BUFFER_SIZE ,sizeof(rc_bmp_data_t) );
	
  // Must initialize callback and imu after dmp buffers are initialized or else we segfault
  if (initialize_imu() == -1) {return -1;}
  while(rc_get_state()!=EXITING){
    rc_usleep(1000000/BMP_CHECK_HZ);
    // perform the i2c reads to the sensor, on bad read just try later
    if(rc_bmp_read(&bmp_data)) continue;
    cbuffer_put(bmp_buffer,&bmp_data);
	
	
  }
  rc_mpu_power_off();
  rc_bmp_power_off();
  
  // free the bmp and mpu buffer
  cbuffer_free(bmp_buffer);
  cbuffer_free(mpu_buffer);
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
 
int get_oldest_mpu_data(rc_mpu_data_t * data){
  return cbuffer_get(mpu_buffer, data); 
}

int get_oldest_bmp_data(rc_bmp_data_t* data){
  return cbuffer_get(bmp_buffer, data); 
}
