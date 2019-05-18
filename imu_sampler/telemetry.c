#include <stdio.h>
#include <robotcontrol.h>
#include <rc/mavlink_uart.h>
#include "circular_buffer.h"
#include "gps.h"
#include "telemetry.h"

static int telemetry_thread_ret_val;
static int bus;
static gps_pvt_t latest;
static char print_buf[128];

static void __callback_func_any()
{
	int sysid = rc_mav_get_sys_id_of_last_msg_any();
	int msg_id = rc_mav_msg_id_of_last_msg();
	
	printf("Message from sysid %d, message %d\n", sysid, msg_id);

}

static int initTelemetry()
{
	rc_pinmux_set(GPS_HEADER_PIN_3, PINMUX_UART);
	rc_pinmux_set(GPS_HEADER_PIN_4, PINMUX_UART);

	//if (rc_uart_init(bus, 57600, 0.5, 0, 1, 0))
	if (rc_mav_uart_init(1, bus, 500000))
	{
		printf("ERROR: failed to initialize uart\n");
		return -1;
	}

	rc_mav_set_callback_all(__callback_func_any);
	rc_uart_flush(bus);

	return 0;
}

int telemetry_main(int uart_bus)
{
	bus = uart_bus;
	if (initTelemetry())
	{
		printf("ERROR: Failed to inittialize telemtry\n");
	}

	size_t bytes = sprintf(print_buf, "Time \t\tLat \t\tLong \t\tAlt \n");
	printf("%s", print_buf);
	rc_uart_write(bus,(uint8_t*) print_buf, bytes);
	

	while (rc_get_state() != EXITING)
	{
		if (get_latest_pvt(&latest))
		{
			printf("ERROR: Faild to get gps data\n");
		}

/*		if (get_latest_dsm(&latest))
		{
			printf("ERROR: Faild to get gps data\n");
		}
		
		if (get_latest_mpu(&latest))
		{
			printf("ERROR: Faild to get gps data\n");
		}
*/		
		size_t bytes = sprintf(print_buf, "%f\t%f\t%f\t%f\n",0.0,latest.latitude, latest.longitude, latest.altitude);
		rc_uart_write(bus,(uint8_t*) print_buf, bytes);
		printf("%s", print_buf);
//		rc_mav_send_heartbeat_abbreviated();
//		rc_mav_send_global_position_int(latitude, longitude, alt, relative_alt, vx++, vy, vz, hdg);
//		rc_mav_send_attitude(0, 0, 90, 0, 0, 0);
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
