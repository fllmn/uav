#include <robotcontrol.h>
#include <math.h>
#include <endian.h>
#include "airspeed.h"
#include "circular_buffer.h"
#include <stdio.h>
#include "sys_log.h"

 #define max(a,b) \
	 ({ __typeof__ (a) _a = (a); \
	    __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

#define MS_PRESSURE_256 0x40
#define MS_PRESSURE_512 0x42
#define MS_PRESSURE_1024 0x44
#define MS_PRESSURE_2048 0x46
#define MS_PRESSURE_4096 0x48
#define MS_TEMPERTURE_256 0x50
#define MS_TEMPERTURE_512 0x52
#define MS_TEMPERTURE_1042 0x54
#define MS_TEMPERTURE_2048 0x56
#define MS_TEMPERTURE_4096 0x58

#define REG_PROM_BASE 0xA0
#define AIRSPEED_BUFFER_SIZE 30
#define I2C_BUS 1
#define MS5525_ADDRESS 0x76
#define RHO_AIR 1.225
static uint64_t init_time;
static int airspeed_thread_ret_val;
static FILE *airspeed_log;
static double mean_pressure_offset = 0;
static uint16_t mean_samples = 0;
cbuffer_handle_t airspeed_buffer;
static uint16_t prom[8];


static int init_airspeed_log()
{
    airspeed_log = fopen("airspeed.log", "w");
    if (airspeed_log == NULL)
    {
        LOG_E("Could not open an AIRSPEED LOG file");
        return -1;
    }

    fprintf(airspeed_log, "time \tairspeed \tpressure \ttemperature \tpressure_raw \ttemperature_raw\n");
    return 0;
}

static int close_airspeed_log()
{
    fclose(airspeed_log);
    return 0;
}

static int read_sens_prom()
{
	uint16_t val;
	for(int i = 0; i < 8;i++)
	{
		if(rc_i2c_read_bytes(I2C_BUS, REG_PROM_BASE+2*i, 2, (uint8_t*) &val) == 2)
		{
			prom[i] = be16toh(val);
		}
		else
		{
			LOG_E("Failed to read prom");
			return -1;
		}


	}
	return 0;
}

static int initilize_airspeed()
{
    airspeed_buffer = create_cbuffer();

    if (airspeed_buffer == NULL)
    {
        LOG_E("Failed to create airspeed buffer");
        return -1;
    }

    if (cbuffer_init(airspeed_buffer, AIRSPEED_BUFFER_SIZE, sizeof(airspeed_t)))
    {
        LOG_E("Failed to allocate airspeed buffer");
        return -1;
    }

    if (rc_i2c_init(I2C_BUS, MS5525_ADDRESS))
    {
        LOG_E("Could not open I2C");
        return -1;
    }


    rc_i2c_send_byte(I2C_BUS, 0x1E);
    rc_usleep(2000000);
    if (read_sens_prom())
    {
    	LOG_E("Failed to read PROM");
	return -1;
    }

    
    init_time = rc_nanos_since_boot();
    return 0;
}

static uint64_t time_since_start()
{
    uint64_t delta_time = rc_nanos_since_boot() - init_time;
    LOG_I("rc_nanos %llu, init_time %llu", rc_nanos_since_boot(), init_time);

  /*  if (delta_time < 0)
    {
        
    }*/

    return (uint64_t) delta_time;
}

static int convert(uint32_t D1, uint32_t D2, double *pressure, double * temperature)
{
	const uint8_t Q1 = 15;
	const uint8_t Q2 = 17;
	const uint8_t Q3 = 7;
	const uint8_t Q4 = 5;
	const uint8_t Q5 = 7;
	const uint8_t Q6 = 21;

	int64_t SENST1 = (int64_t) prom[1];
	int64_t OFFT1 = (int64_t) prom[2];
	int64_t TCS = (int64_t) prom[3];
	int64_t TCO = (int64_t) prom[4];
	int64_t TREF = (int64_t) prom[5];
	int64_t TEMPSENS = (int64_t) prom[6];


	int64_t dT = D2 - (TREF<<Q5);
	int64_t TEMP = 2000 + dT*TEMPSENS/2097152;
	int64_t OFF =  (OFFT1<<Q2) + ((TCO*dT)>>Q4);
	int64_t SENS = (SENST1<<Q1) + ((TCS*dT)>>Q3);
	int64_t P = ((int64_t)D1*SENS/2097152-OFF)/32768;
	const double PSI_to_Pa = 6894.757f;
	*pressure = fabs(PSI_to_Pa * 1.0e-4 * P);
	*temperature = TEMP * 0.01;

	return 0;
}

static uint32_t get_conversion(uint8_t command)
{
	rc_i2c_send_byte(I2C_BUS, command);

	switch (command & 0x0F)
	{
		case 0x08:
			rc_usleep(5600);
		case 0x06:
			rc_usleep(2800);
		case 0x04:
			rc_usleep(1400);
		case 0x02:
			rc_usleep(700);
		case 0x00:
			rc_usleep(700);
			break;
	}

	
	uint8_t val[3];

	int bytes = rc_i2c_read_bytes(I2C_BUS, 0, 3, val);
	if (bytes != 3)
	{
		printf("Error no adc value");
		return 0;
	}

	return (val[0] << 16) |  (val[1] << 8) | val[2];
}

static int calculate_airspeed(double pressure, double *airspeed)
{
	*airspeed = sqrt(2*pressure/RHO_AIR);
	return 0;	
}


void log_airspeed()
{
    if (airspeed_log == NULL)
    {
        LOG_E("Tried to write to uninitilized log file");
        return;
    }

    airspeed_t d;
    if (cbuffer_try_get(airspeed_buffer, &d))
    {
        LOG_W("LOG not available");
    }
    else
    {
        fprintf(airspeed_log, "%lld, %f, %f, %f, %d, %d\n",
                d.time,
                d.airspeed,
                d.pressure,
                d.temperature,
                d.pressure_raw,
                d.temperature_raw);
    }
    return;
}

int get_latest_airspeed(airspeed_t *latest)
{
    if (airspeed_buffer != NULL)
    {
        if (cbuffer_top(airspeed_buffer, latest))
        {
            LOG_E("Failed to peek buffer");
            return -1;
        }
    }

    return 0;
}

int airspeed_main()
{
    if (initilize_airspeed())
    {
        LOG_C("Failed to initilize airspeed sensor");
    }


    init_airspeed_log();
    uint32_t D1;
    uint32_t D2;
    double pressure;
    double temperature;
    double airspeed;
    while(rc_get_state() != EXITING)
    {
        D1 = get_conversion(MS_PRESSURE_4096);
        D2 = get_conversion(MS_TEMPERTURE_4096);

        convert(D1, D2, &pressure, &temperature);
        if (mean_samples < 100)
        {
            if (mean_pressure_offset > 0.00001)
            {
                // Calculate cumulative mean
                mean_pressure_offset = (double)(mean_samples-1)/((double)mean_samples) * mean_pressure_offset + pressure/((double) mean_samples);
		mean_samples++;
            }
            else 
            {
                mean_pressure_offset = pressure;
                mean_samples++;
            }

            airspeed = 0;

        }
        else
        {
            pressure = max(pressure - mean_pressure_offset, 0);
            calculate_airspeed(pressure, &airspeed);
        }

        airspeed_t d;
        d.time = rc_nanos_since_boot();
        d.airspeed = airspeed;
        d.pressure = pressure;
        d.temperature = temperature;
        d.pressure_raw = D1;
        d.temperature_raw = D2;

        cbuffer_put(airspeed_buffer, &d);
	//LOG(LOG_INFO, "Time %llu Pdiff %f [Pa] Temp %f [C] Airspeed %f [m/s] D1 %x D2 %x",d.time, pressure, temperature, airspeed, D1, D2);
        rc_usleep(200000);

    }

    close_airspeed_log();
    return -1;
}

void *airspeed_thread_func()
{
    airspeed_thread_ret_val = airspeed_main();
    if (airspeed_thread_ret_val == -1)
    {
        rc_set_state(EXITING);
    }

    return (void*)&airspeed_thread_ret_val;
}
