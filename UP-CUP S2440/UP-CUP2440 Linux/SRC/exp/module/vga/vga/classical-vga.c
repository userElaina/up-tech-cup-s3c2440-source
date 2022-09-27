#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#define I2C_RETRIES     0x0701
#define I2C_TIMEOUT     0x0702
#define I2C_RDWR        0x0707 

struct i2c_msg {        
	__u16 addr;     /* slave address                        */        
	__u16 flags;
	__u16 len;
	__u8 *buf;      /* pointer to msg data                  */
};

struct i2c_rdwr_ioctl_data {        
	struct i2c_msg *msgs;   /* pointers to i2c_msgs */        
	int nmsgs;              /* number of i2c_msgs   */
}; 

int main(int argc, char **argv)
{  
	struct i2c_rdwr_ioctl_data data;  
	struct i2c_msg msg;
	unsigned char buff[8] = {0x04, 0x20, 0x07, 0x8c, 0x0e, 0x1b, 0x0d, 0x03};

	unsigned char readbuf = 0x25;
	int ret;   
	int fd;


	fd = open("/dev/i2c-0", O_RDWR);   
	if (!fd) 
	{  	
		printf("Error on opening the device file\n");  
		return 0;  
	}  

	data.nmsgs = 1;   
	data.msgs = &msg;  

	msg.addr = 0x76;
	msg.flags = 0;
	msg.len = 8;
	msg.buf = buff; 
	
	ioctl(fd,I2C_TIMEOUT,5);  /* set the timeout    */  
	ioctl(fd,I2C_RETRIES,5);  
	ioctl(fd, I2C_RDWR, &data);

	msg.len = 1;
	msg.buf = &readbuf;
	ioctl(fd, I2C_RDWR, &data);

	msg.flags = 1;
	ioctl(fd, I2C_RDWR, &data);
	printf("Version ID is 0x%02x\n", readbuf);

	readbuf = 0x04;
	msg.flags = 0;
	ioctl(fd, I2C_RDWR, &data);

	msg.flags = 1;
	ioctl(fd, I2C_RDWR, &data);
	printf("Address 0x04 is 0x%02x\n", readbuf);

	readbuf = 0x07;
	msg.flags = 0;
	ioctl(fd, I2C_RDWR, &data);

	msg.flags = 1;
	ioctl(fd, I2C_RDWR, &data);
	printf("Address 0x07 is 0x%02x\n", readbuf);

	readbuf = 0x0e;
	msg.flags = 0;
	ioctl(fd, I2C_RDWR, &data);

	msg.flags = 1;
	ioctl(fd, I2C_RDWR, &data);
	printf("Address 0x0e is 0x%02x\n", readbuf);

	readbuf = 0x0d;
	msg.flags = 0;
	ioctl(fd, I2C_RDWR, &data);

	msg.flags = 1;
	ioctl(fd, I2C_RDWR, &data);
	printf("Address 0x0d is 0x%02x\n", readbuf);
	close(fd);  
	return;
}
