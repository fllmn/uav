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
static uint32_t custom_mode;
static uint8_t type;
static uint8_t autopilot;
static uint8_t base_mode;
static uint8_t system_status;
static int initTelemetry()
{
    rc_pinmux_set(UART1_HEADER_PIN_3, PINMUX_UART);
    rc_pinmux_set(UART1_HEADER_PIN_4, PINMUX_UART);

#ifndef MAVLINK
    if (rc_uart_init(bus, 57600, 0.5, 0, 1, 0))
    {
        LOG_E(" Failed to initialize uart");
        return -1;
    }

    rc_uart_flush(bus);
#else
    if (rc_mav_init(1, 1, 50000))
    {
        LOG_E(" Failed to initilize MAVLINK");
        return -1;
    }
#endif
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

    sprintf(print_buf, "Time \t\tLat \t\tLong \t\tAlt \n");

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

#ifndef MAVLINK
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
#else
        rc_mav_send_heartbeat(custom_mode, type, autopilot, base_mode, system_status);
        rc_mav_send_global_position_int((int32_t) latest_pvt.latitude*1E7,
                                        (int32_t) latest_pvt.longitude*1E7,
                                        (int32_t) latest_pvt.altitude*1E3,
                                        (int32_t) latest_pvt.altitude*1E3,
                                        0,
                                        0,
                                        0,
                                        0);
        rc_uart_write(bus,(uint8_t*) print_buf, bytes);
        rc_usleep(1000000);
#endif
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
