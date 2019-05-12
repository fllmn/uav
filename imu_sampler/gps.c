#include <stdio.h>
#include <robotcontrol.h>
#include "ubx.h"

#define BAUDRATE 9600
#define TIMEOUT 0.5
#define CANONICAL 0
#define STOP_BITS 1
#define PARITY 0
#define READ_SIZE 128

struct uart_conf{
    int bus;
    int baudrate;
    float timeout;
    int canonical_en;
    int stop_bits;
    int parity_en;
    char buf[256];
} uart_conf;

static void setup_uart()
{
    rc_uart_init(uart_conf.bus, uart_conf.baudrate, uart_conf.timeout, uart_conf.canonical_en, uart_conf.stop_bits, uart_conf.parity_en);
    rc_uart_flush(uart_conf.bus);
}

static void disable_nmea_message(int bus, char *message)
{
    char tx_buf[29];

    tx_buf[0] = (uint8_t) '$';
    tx_buf[1] = (uint8_t) 'P';
    tx_buf[2] = (uint8_t) 'U';
    tx_buf[3] = (uint8_t) 'B';
    tx_buf[4] = (uint8_t) 'X';
    tx_buf[5] = (uint8_t) ',';
    tx_buf[6] = (uint8_t) '4';
    tx_buf[7] = (uint8_t) '0';
    tx_buf[8] = (uint8_t) ',';
    tx_buf[12] = (uint8_t) ',';
    tx_buf[13] = (uint8_t) '0';
    tx_buf[14] = (uint8_t) ',';
    tx_buf[15] = (uint8_t) '0';
    tx_buf[16] = (uint8_t) ',';
    tx_buf[17] = (uint8_t) '0';
    tx_buf[18] = (uint8_t) ',';
    tx_buf[19] = (uint8_t) '0';
    tx_buf[20] = (uint8_t) ',';
    tx_buf[21] = (uint8_t) '0';
    tx_buf[22] = (uint8_t) ',';
    tx_buf[23] = (uint8_t) '0';
    tx_buf[24] = (uint8_t) '*';

    memcpy(&tx_buf[9], message, sizeof(char)*3);

    uint16_t checksum = 0;

    for (int i = 1; i < 24;i++)
    {
        checksum ^= tx_buf[i];
    }

    sprintf(&tx_buf[25], "%02X", checksum);

    tx_buf[27] = (uint8_t) '\r';
    tx_buf[28] = (uint8_t) '\n';

    printf("Send_bdduf %s\n",tx_buf);

    rc_uart_write(bus,(uint8_t*) tx_buf, 29);	

}

int initialize_gps(int bus)
{
    if (!(bus==1||bus==2))
    {
        printf(stderr, "ERROR: illegal bus number");
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

    return 0;
}



int gps_main(int bus)
{
    if (initialize_gps(bus) == -1)
    {
        printf(stderr, "ERROR: failed to initialize uart");
        return -1;
    }

    size_t rem_data = 0;
    while(rc_get_state() != EXITING)
    {
        while (rc_uart_bytes_available(uart_conf.bus) > READ_SIZE)
        {

            rc_uart_read_bytes(uart_conf.bus, uart_conf.buf[rem_data], READ_SIZE-1);
            rem_data = READ_SIZE-1;

            switch(process_buffer(uart_conf.buf, &rem_data))
            {
            case NAV:
                printf("Got nav package");
                //getPosition()
                break;
            case UNKNOWN:
                printf("Got UNKNOWN package");
                break;
            }
        }
    }

    return 0;
}

