/*
	fpga-test.c, need insmod s3c2440-fpga.o first.
	author: wb <wbinbuaa@163.com>
	date:   2005-6-13 21:05
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

//FPGA is on bank 4
#define oFPGA_IOP1L		0
#define oFPGA_IOP1H		2
#define oFPGA_IOP2L		4
#define oFPGA_IOP2H		6
#define oFPGA_IOP3L		8
#define oFPGA_IOP3H		0xA
#define oFPGA_LED_CTRL		0xC
#define oFPGA_RESET		0x3E

static void help_menu()
{
	printf("\n");
	printf("DESCRIPTION\n");
	printf("      Fpga test program. Writing the register of the fpga loaded in        \n");	
	printf("      the Bank4, it can control the frequency of leds on the fpga board. \n");	
	printf("OPTIONS\n");
	printf("      -h or --help:    this menu\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	int fd;
	unsigned char *iobase;
	int cnt=0,tn=0;
	char ch;
	fd = open("/dev/fpga",O_RDWR);
	if(fd < 0){
		printf("####fpga  device open fail####\n");
		return (-1);
	}

	iobase = (unsigned char *)mmap(0, 4096,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
	usleep(80000);
	cnt=10;
	ioctl(fd,oFPGA_LED_CTRL,cnt);

                fflush(stdout);
	
	
	close(fd);
	return 0;
}
