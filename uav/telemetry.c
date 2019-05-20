#include <stdio.h>
#include <robotcontrol.h>
#include "sys_log.h"
#include "circular_buffer.h"
#include "gps.h"
#include "baro.h"
#include "imu.h"
#include "telemetry.h"

static int telemetry_thread_ret_val;
static int bus;
static gps_pvt_t latest_pvt;
static bmp_entry_t latest_baro;
static imu_entry_t latest_imu;
static char print_buf[1024];

static int initTelemetry()
{
    rc_pinmux_set(UART1_HEADER_PIN_3, PINMUX_UART);
    rc_pinmux_set(UART1_HEADER_PIN_4, PINMUX_UART);

    if (rc_uart_init(bus, 57600, 0.5, 0, 1, 0))
    {
        LOG_E(" Failed to initialize uart");
        return -1;
    }

    rc_uart_flush(bus);

    return 0;
}

int telemetry_main(int uart_bus)
{
    bus = uart_bus;
    if (initTelemetry())
    {
        LOG_E("Failed to inittialize telemtry");
    }

    LOG_I("Initilized telemtry thread");

    size_t bytes = sprintf(print_buf, "Time \t\tLat \t\tLong \t\tAlt \n");

    while (rc_get_state() != EXITING)
    {
        if (get_latest_pvt(&latest_pvt))
        {
            LOG_E("Failed to get gps data");
        }

        if (get_latest_baro(&latest_baro))
        {
            LOG_E("Failed to get baro data");
        }

        if (get_latest_imu(&latest_imu))
        {
            LOG_E("Failed to get imu data");
        }

        size_t bytes = sprintf(print_buf, "%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f \t%5f\n",
                               latest_imu.accel[0], latest_imu.accel[1], latest_imu.accel[2],
                               latest_imu.gyro[0], latest_imu.gyro[1], latest_imu.gyro[2],
                               latest_imu.mag[0], latest_imu.mag[1], latest_imu.mag[2],
                               latest_imu.temp,
                               latest_imu.euler[0], latest_imu.euler[1], latest_imu.euler[2],
                               latest_baro.bmp_data.temp_c,
                               latest_baro.bmp_data.alt_m,
                               latest_baro.bmp_data.pressure_pa,
                               latest_pvt.latitude,
                               latest_pvt.longitude,
                               latest_pvt.altitude);
        rc_uart_write(bus,(uint8_t*) print_buf, bytes);
        rc_usleep(1000000);
    }

    return 1;
}


void *telemetry_thread_func()
{
    telemetry_thread_ret_val = telemetry_main(1);
    if (telemetry_thread_ret_val == -1)
    {
        rc_set_state(EXITING);
    }

    return (void*) &telemetry_thread_ret_val;
}
