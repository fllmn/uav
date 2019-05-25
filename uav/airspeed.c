#include <robotcontrol.h>
#include "airspeed.h"
#include "circular_buffer.h"
#include "sys_log.h"


#define AIRSPEED_BUFFER_SIZE 30
#define I2C_BUS 1
#define MS5525_ADDRESS 0x76
static uint64_t init_time;
static int airspeed_thread_ret_val;
static FILE *airspeed_log;
static double mean_pressure_offset = 0;
static uint16_t mean_samples = 0;
cbuffer_handle_t airspeed_buffer;


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

static int initilize_airspeed()
{
    airspeed_buffer = create_cbuffer();

    if (airspeed_buffer == NULL)
    {
        LOG_C("Failed to create airspeed buffer");
        return -1;
    }

    if (cbuffer_init(airspeed_buffer, AIRSPEED_BUFFER_SIZE, sizeof(airspeed_t)))
    {
        LOG_C("Failed to allocate airspeed buffer");
        return -1;
    }

    if (rc_i2c_init(I2C_BUS, MS5525_ADDRESS))
    {
        LOG_C("Could not open I2C");
        return -1;
    }

    init_time = rc_nanos_since_epoch();

    return 0;
}

static uint64_t time_since_start(uint64_t time)
{
    uint64_t time_now = rc_nanos_since_epoch()
    int64_t delta_time = time_now - time;

  /*  if (delta_time < 0)
    {
        
    }*/
}

int log_airspeed()
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
                d.tempwerature,
                d.pressure_raw,
                d.temperature_raw);
    }
    return 0;
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
        if (time_since_start(init_time) < 1000000000)
        {
            if (mean_pressure_offset != 0)
            {
                // Calculate cumulative mean
                mean_pressure_offset = (double)(mean_samples-1)/((double)mean_samples) * mean_pressure_offset + pressure/((double) mean_samples);
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
            pressure = pressure - mean_pressure_offset;
            calculate_airspeed(pressure, temperture, &airspeed);
        }

        airspeed_t d;
        d.time = rc_nanos_since_epoch();
        d.airspeed = airspeed;
        d.pressure = pressure;
        d.temperture = termperture;
        d.pressure_raw = D1;
        d.temperture_raw = D2;

        cbuffer_put(airspeed_buffer, &d);

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
