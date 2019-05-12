#include "i2c_thread.h"
// Global variables for IMU/DMP
static int i2c_thread_ret_val;
#define MPU_BUFFER_SIZE   200
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


//global vars for bmp (barometer)
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
 
