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

