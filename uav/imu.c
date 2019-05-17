#include <stdio.h>
#include "imu.h"
#include "circular_buffer.h"
#define IMU_BUFFER_SIZE   10

// a wraper struct of  rc_mpu_data_t with the extra time field
typedef struct imu_entry_t {
  	uint64_t time_ns;
	double accel[3];
	double gyro[3];
	double mag[3];
	double temp;
	double euler[3];
} imu_entry_t;


rc_mpu_config_t mpu_config; 
rc_mpu_data_t  	mpu_data; // Declare a Motion processing data structure, this is used to receive data from imu interrupt

imu_entry_t  	imu_entry; // Declare a imu_buffer_entry
cbuffer_handle_t imu_buffer; //Declare a cbuffer
FILE *imu_log;


int init_imu_log()
{
	// add flag for enable and disable logging?
	imu_log = fopen("imu.log","w");

	if(imu_log == NULL)
   {
		printf("Could not open an IMU log file!\n");   
		return -1;             
   }
   fprintf(imu_log,"time acc_x acc_y acc_z p q r mag_x mag_y mag_z temp roll pitch yaw\n");
	return 0;
}

int close_imu_log()
{
	// add flag for enable and disable logging?	
	fclose(imu_log);
	return 0;
}


// IMU/DMP functions
void sample_imu()
{
	rc_mpu_block_until_dmp_data();
  // When this callback is called we know a new set of mpu_data has been sampled
    imu_entry.time_ns = rc_nanos_since_epoch(); // set the timestamp should probably correct with rc_mpu_nanos_since_last_dmp_interrupt()
	memcpy(&imu_entry.accel,&mpu_data.accel,sizeof(double)*3);
	memcpy(&imu_entry.gyro,&mpu_data.gyro,sizeof(double)*3);
	memcpy(&imu_entry.mag,&mpu_data.mag,sizeof(double)*3);
	
	imu_entry.temp = mpu_data.temp;
	
	memcpy(&imu_entry.euler,&mpu_data.dmp_TaitBryan,sizeof(double)*3);
	

	cbuffer_put(imu_buffer,&imu_entry);
	
}

int get_oldest_mpu_data(imu_entry_t * data)
{
	return cbuffer_get(imu_buffer, data); 
}

int initialize_imu()
{

	// Init logfile
	if (init_imu_log()) return -1;
	
	// Init mpu buffer
	imu_buffer = create_cbuffer();
	cbuffer_init(imu_buffer,IMU_BUFFER_SIZE ,sizeof(imu_entry_t) );
		
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
	
	// Initialize with config and pointer to data structure
	if (rc_mpu_initialize_dmp(&mpu_data,mpu_config))
    {
		fprintf(stderr,"ERROR: failed to initialize mpu in dmp mode\n");
		return -1;
    }
	
	return 0;
	
}

int finalize_imu()
{
	int r1 = rc_mpu_power_off();
	int r2 = cbuffer_free(imu_buffer);
	close_imu_log();
	if (r1 || r2) return -1;
	return 0;
}

int log_imu(){
	
	if(imu_log == NULL)
	{
		printf("Tried to write before imu log file was created\n");   
		return -1;             
	}
	imu_entry_t d;
	if(cbuffer_try_get(imu_buffer,&d))
	{
		//Log was empty
		return -1;
	}
   
   
	fprintf(imu_log,"%llu, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f;\n",
	d.time_ns,
	d.accel[0], d.accel[1], d.accel[2],
	d.gyro[0], d.gyro[1], d.gyro[2],
	d.mag[0], d.mag[1], d.mag[2],
	d.temp,
	d.euler[0], d.euler[1], d.euler[2]);
	return 0;
	
	
}


