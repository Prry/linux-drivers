
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h> 
#include <sys/ioctl.h>
#include "bmp180.h"

int main(int argc, int **argv)
{
	int fd;
	uint8_t buf[10];
	short temp = 0;
	long pre = 0;
	char chip_id=0x00;
	
	fd = open("/dev/bmp180", O_RDWR);
	if(-1 == fd)
	{
		perror("open bmp180 failed\n");
		return -1;
	}
	ioctl(fd, BMP_GET_CHIP_ID, &chip_id); 
	printf("open bmp180 driver succ, chip ID[%x]\n", chip_id);
	for(;;)
	{
		read(fd, buf, 10);
		temp = (buf[1]<<8)|buf[0];
		pre  = *(long*)&buf[2];
		printf("current temperature[%.1f C],pressure[%ld Pa]\n", ((float)temp)/10, pre);
		sleep(2);
	}
	close(fd);
	
	return 0;
}

