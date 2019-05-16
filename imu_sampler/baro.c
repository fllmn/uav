#include "baro.h"
#include "circular_buffer.h"
//global vars for bmp (barometer)
#define		BMP_BUFFER_SIZE	  10
#define 	BMP_CHECK_HZ      5
rc_bmp_data_t   bmp_data; // Declare a Barometer struct
cbuffer_handle_t bmp_buffer; //Declare a cbuffer

int initialize_baro(){
	
	if(rc_bmp_init(BMP_OVERSAMPLE_16, BMP_FILTER_OFF))    return -1;
	if(rc_bmp_read(&bmp_data)) return -1;
	
	// Init bmp buffer
	bmp_buffer = empty_cbuffer();
	return cbuffer_init(bmp_buffer,BMP_BUFFER_SIZE ,sizeof(rc_bmp_data_t) );
	

}

int finalize_baro(){
	int r1  = rc_bmp_power_off();
  
	// free the bmp and mpu buffer
	int r2  = cbuffer_free(bmp_buffer);
	if (r1||r2) return -1;
	return 0;
}

void sample_baro(){
		// perform the i2c reads to the sensor, on bad read just try later
		if(rc_bmp_read(&bmp_data)) return; // continue jumps back to the whilk statement
		cbuffer_put(bmp_buffer,&bmp_data);

}

int baro_sample_time(){
	return  1000000/BMP_CHECK_HZ;
}

int get_oldest_bmp_data(rc_bmp_data_t* data){
	return cbuffer_get(bmp_buffer, data); 
}