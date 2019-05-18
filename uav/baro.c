#include "baro.h"
#include "circular_buffer.h"
#include <stdio.h>
//global vars for bmp (barometer)
#define		BMP_BUFFER_SIZE	  10

cbuffer_handle_t bmp_buffer; //Declare a cbuffer

bmp_entry_t bmp_entry;

FILE *baro_log;


int init_baro_log()
{
    // add flag for enable and disable logging?
    baro_log = fopen("baro.log","w");

    if(baro_log == NULL)
    {
        printf("Could not open an Barometer log file!\n");
        return -1;
    }
    fprintf(baro_log,"time temp_c alt_m pressure_pa\n");
    return 0;
}

int close_baro_log()
{
    // add flag for enable and disable logging?
    fclose(baro_log);
    return 0;
}


int log_baro(){

    if(baro_log == NULL)
    {
        printf("Tried to write before baro log file was created\n");
        return -1;
    }
    bmp_entry_t d;

    if(cbuffer_try_get(bmp_buffer,&d))
    {
        //Log was empty
        return -1;
    }


    fprintf(baro_log,"%llu, %f, %f, %f;\n",
            d.time_ns,
            d.bmp_data.temp_c,
            d.bmp_data.alt_m,
            d.bmp_data.pressure_pa
           );
    return 0;
}

int initialize_baro(){

    if (init_baro_log()) return -1;

    if(rc_bmp_init(BMP_OVERSAMPLE_16, BMP_FILTER_OFF))    return -1;
    if(rc_bmp_read(&bmp_entry.bmp_data)) return -1;
    // Init bmp buffer
    bmp_buffer = create_cbuffer();
    return cbuffer_init(bmp_buffer,BMP_BUFFER_SIZE ,sizeof(bmp_entry_t) );


}

int finalize_baro(){
    int r1  = rc_bmp_power_off();
    // free the bmp and mpu buffer
    int r2  = cbuffer_free(bmp_buffer);
    close_baro_log();
    if (r1||r2) return -1;
    return 0;
}

int get_latest_baro(bmp_entry_t *baro)
{
    if(bmp_buffer != NULL)
    {
        if(cbuffer_top(bmp_buffer, baro))
        {
            printf("ERROR: Failed to peek bruffer");
            return -1;
        }
    }
    return 0;
}

void sample_baro(){
    // perform the i2c reads to the sensor, on bad read just try later
    if(rc_bmp_read(&bmp_entry.bmp_data)) return; // continue jumps back to the while statement

    bmp_entry.time_ns= rc_nanos_since_epoch();//timestamp
    cbuffer_put(bmp_buffer,&bmp_entry); // put on buffer

}

// dont know which data type i want to use yet. The wrapper one or the original?
/* int get_oldest_bmp_data(bmp_entry_t* data){
   return cbuffer_get(bmp_buffer, data);
   } */
