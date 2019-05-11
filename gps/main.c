/**
 * @file rc_uart_loopback.c
 * @example    rc_uart_loopback
 *
 * This is a test to check read and write operation of UART buses. For this
 * example to work, connect the RX and TX wires of one of the included 4-pin
 * JST-SH pigtails and plug into the UART1 or UART5 headers. You may also elect
 * to test UART0 on the debug header or UART2 on the GPS header. The test
 * strings this programs transmits will then loopback to the RX channel.
 */

#include <stdio.h>
#include <stdlib.h> // for atoi
#include <string.h>
#include <unistd.h>
#include <rc/uart.h>

#define BUF_SIZE	32
#define TIMEOUT_S	0.5
#define BAUDRATE	9600

static void disable_message(int bus, char *message)
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

static void __print_usage(void)
{
	printf("\n");
	printf("Usage: rc_uart_loopback {bus}\n");
	printf("This sends a short message out the specified bus and then\n");
	printf("reads it back in. This requires connecting RX to TX to make a loopback.\n");
	printf("For Robotics Cape or BeagleBone Blue specify bus 0,1,2 or 5\n");
	printf("\n");
	return;
}

int main(int argc, char *argv[])
{
	uint8_t buf[BUF_SIZE];
	int bus; // which bus to use
        FILE *fp;

	// Parse arguments
	if(argc!=2){ //argc==2 actually means one argument given
		__print_usage();
		return -1;
	}
	else
        {
            bus = atoi(argv[1]);
        }

	if(!(bus==0||bus==1||bus==2||bus==5)){
		__print_usage();
		return -1;
	}

	printf("\ntesting UART bus %d\n\n", bus);
	// disable canonical (0), 1 stop bit (1), disable parity (0)
	if(rc_uart_init(bus, BAUDRATE, TIMEOUT_S, 0,1,0)){
		printf("Failed to rc_uart_init%d\n", bus);
		return -1;
	}

        fp = fopen("/home/debian/rc/log.txt", "w");

        if (fp == NULL)
        {
            printf("Failed to open file");
            return -1;
        }

	disable_message(bus, "GSV");
	disable_message(bus, "GSV");
	disable_message(bus, "RMC");
	disable_message(bus, "VTG");
	disable_message(bus, "GGA");
	
        while(1)
        {
            while(rc_uart_bytes_available(bus) > BUF_SIZE)
            {
                rc_uart_read_bytes(bus, buf, BUF_SIZE-1);
                fputs((char*)buf, fp);
		printf("%s", buf);
            }
            usleep(100);
        }

	// close
	return 0;
}
