#include <stdio.h>
#include <robotcontrol.h>
#include <rc/gpio.h>
#include "sys_log.h"
#include "gps.h"
#include "ubx.h"
#include "circular_buffer.h"

#define BAUDRATE 9600
#define TIMEOUT 0.5
#define CANONICAL 0
#define STOP_BITS 1
#define PARITY 0
#define READ_SIZE 96

static int gps_thread_ret_val;
#define GPS_BUFFER_SIZE 30
unsigned int gps_buffer_idx = 0;
static bool has_pps = false;
static int pps_pin = -1;
static uint64_t event_time = 0;
cbuffer_handle_t gps_buffer;
static FILE *gps_log;

struct uart_conf{
    int bus;
    int baudrate;
    float timeout;
    int canonical_en;
    int stop_bits;
    int parity_en;
    char buf[512];
} uart_conf;

static int init_gps_log()
{
    gps_log = fopen("gps.log", "w");
    if (gps_log == NULL)
    {
        LOG_E("Could not open an GPS LOG file!");
        return -1;
    }

    fprintf(gps_log, "time \tlat \tlon \taltitude\n");
    return 0;
}

static int close_gps_log()
{
    fclose(gps_log);
    return 0;
}

static void setup_uart()
{
    rc_pinmux_set(GPS_HEADER_PIN_3,	PINMUX_UART);
    rc_pinmux_set(GPS_HEADER_PIN_4, PINMUX_UART);
    rc_uart_init(uart_conf.bus, uart_conf.baudrate, uart_conf.timeout, uart_conf.canonical_en, uart_conf.stop_bits, uart_conf.parity_en);
    rc_uart_flush(uart_conf.bus);
}

static void disable_nmea_message(int bus, char *message)
{
    char* pubx_mess = "$PUBX,40,XXX,0,0,0,0,0,0*XX\r\n";
    memcpy(&pubx_mess[9], message, sizeof(char)*3);

    uint16_t checksum = 0;

    for (int i = 1; i < 24;i++)
    {
        checksum ^= pubx_mess[i];
    }

    sprintf(&pubx_mess[25], "%02X", checksum);

    rc_uart_write(bus,(uint8_t*) pubx_mess, 29);
}

void enable_ubx_nav()
{
    uint8_t tx_buf[128];

    size_t mess_size = 0;

    get_nav_enable_mess(tx_buf, &mess_size);

    rc_uart_write(uart_conf.bus,  tx_buf, mess_size);
}

int initialize_gps(int bus)
{
    if (init_gps_log()) return -1;

    if (!(bus==1||bus==2))
    {
        LOG_E("ERROR: illegal bus number\n");
        return -1;
    }

    uart_conf.bus = bus;
    uart_conf.baudrate = BAUDRATE;
    uart_conf.timeout = TIMEOUT;
    uart_conf.canonical_en = CANONICAL;
    uart_conf.stop_bits = STOP_BITS;
    uart_conf.parity_en = PARITY;

    setup_uart();
    disable_nmea_message(bus, "GSV");
    disable_nmea_message(bus, "GSV");
    disable_nmea_message(bus, "RMC");
    disable_nmea_message(bus, "VTG");
    disable_nmea_message(bus, "GLL");
    disable_nmea_message(bus, "GGA");
    disable_nmea_message(bus, "GSA");

    enable_ubx_nav();

    init_gps_log();
    return 0;
}

int finalize_gps()
{
    int r1 = rc_uart_close(uart_conf.bus);
    int r2 = close_gps_log();

    return (r1|r2);
}

void log_gps()
{
    if (gps_log == NULL)
    {
        LOG_E("Tried to write to uninitialized gps log file");
        return;
    }

    gps_pvt_t d;
    if (cbuffer_try_get(gps_buffer, &d))
    {
        return;
    }

    fprintf(gps_log, "%lu, %f, %f, %f",
            rc_nanos_since_boot(),
            d.latitude,
            d.longitude,
            d.altitude);

    return;
}

static void push_latest()
{
    positionType latestPosition;
    getLatestPosition(&latestPosition);

    gps_pvt_t element;
    element.time_ns = event_time;
    element.latitude = latestPosition.latitude;
    element.longitude = latestPosition.longitude;
    element.altitude = latestPosition.altitude;

    if(cbuffer_put(gps_buffer, &element))
    {
        LOG_E("Failed to put GPS element");
    }
}

int get_latest_pvt(gps_pvt_t *latestPvt)
{
    if (gps_buffer != NULL)
    {
        if (cbuffer_top(gps_buffer, latestPvt))
        {
            printf("ERROR: Failed to peek buffer\n");
            return -1;
        }
    }

    return 0;
}

static int gps_process()
{
    int ret = UNKNOWN;
    size_t rem_data = 0;
    size_t bytes_avail = rc_uart_bytes_available(uart_conf.bus);
    size_t bytes_read = 0;
    while (bytes_avail > 1)
    {

        bytes_read = (bytes_avail > READ_SIZE) ? READ_SIZE : bytes_avail;
        rc_uart_read_bytes(uart_conf.bus,(uint8_t*) &uart_conf.buf[rem_data], bytes_read-1);
        rem_data = rem_data + bytes_read-1;
        bytes_avail =- bytes_read-1;
        switch(process_buffer((uint8_t*)uart_conf.buf, &rem_data))
        {
        case NAV:
            push_latest();
            ret = NAV;
            break;
        case UNKNOWN:
            break;
        default:
            break;
        }
    }
    return ret;

}

int setup_pps_pin(int pps_pin_nbr)
{
    pps_pin = rc_gpio_init_event(0, pps_pin_nbr, 0, RC_GPIOEVENT_RISING_EDGE);

    if (pps_pin == -1)
    {
        LOG_E("Failed to setup event pin", __FUNCTION__);
        return -1;
    }

    has_pps = true;

    return 0;
}

int gps_main(int bus, int pps_pin_nbr)
{
    gps_buffer = create_cbuffer();
    if (gps_buffer == NULL)
    {
        LOG_W("Failed to create gps buffer");
    }

    if (cbuffer_init(gps_buffer, GPS_BUFFER_SIZE, sizeof(gps_pvt_t)))
    {
        LOG_W("Failed to init gps buffer");
    }

    if (initialize_gps(bus) == -1)
    {
        printf("ERROR: failed to initialize uart\n");
        return -1;
    }

    if (pps_pin_nbr != -1)
    {
        if (setup_pps_pin(pps_pin_nbr))
        {
            LOG_W("Failed to setup PPS input");
        }
    }

    while(rc_get_state() != EXITING)
    {
        if (!has_pps)
        {
            event_time = rc_nanos_since_epoch();
            gps_process();
            rc_usleep(10000);
        }
        else
        {
            int event = rc_gpio_poll(0, pps_pin_nbr, 100, &event_time);
            if (event == RC_GPIOEVENT_RISING_EDGE)
            {
                gps_process();
            }
        }
    }

    return 0;
}


void *gps_thread_func()
{
    gps_thread_ret_val = gps_main(2, 17);
    if (gps_thread_ret_val == -1)
    {
        rc_set_state(EXITING);
    }

    return (void*)&gps_thread_ret_val;
}
