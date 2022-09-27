/*
 * s3c2440-485.c
 *
 *  S3C2440 485
 *
 * Author: wb <wbinbuaa@163.com>
 * Date  : $Date: 2005/08/09 $ 
 *
 * $Revision: 1.0.0.1 $
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <asm-arm/arch-s3c2410/map.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm-arm/arch-s3c2410/regs-gpio.h>
#include <asm/plat-s3c/regs-serial.h>

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK(x...) {printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...) (void)(0)
#endif

#define MAX485_PIN_R2S		(1<<1)  //GPIO_H1:set mode receive or send
#define _485_Mode_Rev()         do {writel((readl(dat_addr) & (~MAX485_PIN_R2S)),dat_addr);\
                                udelay(1000);}while(0);
#define _485_Mode_Send()        do {writel((readl(dat_addr) | MAX485_PIN_R2S),dat_addr);\
                                udelay(1000);}while(0);
#define _485_IOCTRL_RE2DE	(0x10)			//send or receive
#define _485_RE			0				//receive
#define _485_DE			1				//send

#define DEVICE_NAME	"s3c2440_485"
#define _485RAW_MINOR	1

#define DEVICE_MAJOR 252
#define DEVICE_MINOR  0

struct cdev *mycdev;
struct class *myclass;
dev_t devno;

static int _485Major = 0;
signed int base_addr;
signed int dat_addr;
signed int base;//for cpld
signed int tmpbak;
static int _485_open(struct inode *inode, struct file *filp)
{
	printk( "s3c2440 485 device open!\n");
	return 0;
}

static int _485_release(struct inode *inode, struct file *filp)
{
	printk( "s3c2440 485 device release!\n");
	return 0;
}


//for 485 iocontrol
static int _485_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	
	switch(cmd){
	case _485_IOCTRL_RE2DE:
		
		if(_485_RE == arg) 
			_485_Mode_Rev()
		else if (_485_DE == arg) 
			_485_Mode_Send()
		break;
	default:
		printk("Do not have this ioctl methods\n");
	}

	return 0;	
}


static struct file_operations s3c2440_fops = {
	owner:	THIS_MODULE,
	open:	_485_open,	
//	read:	_485_read,	
//	write:	_485_write,
	ioctl:	_485_ioctl,
	release:	_485_release,
};


int __init _485_init(void)
{
	int ret;
	dat_addr = ioremap(0x56000074,0x20);
	base_addr = ioremap(0x50008000, 0x20);
	s3c2410_gpio_cfgpin(S3C2410_GPH1,S3C2410_GPH1_OUTP);  
	s3c2410_gpio_pullup(S3C2410_GPH1, 0);   
//	printk("GPHCON:%x\tGPHUP:%x\tGPHDAT:%x\n", S3C2410_GPHCON,S3C2410_GPHUP, S3C2410_GPHDAT);
   
	writel(readl(base_addr) & (~S3C2410_LCON_IRM),base_addr);	
	
	_485Major=ret;

	signed int tmp;

	base = ioremap(0x08000114, 0x400);
	tmp = readb(base);
	tmpbak=tmp;	
	tmp |= 0x02;
	writeb(tmp, base);
	
	int err;
        devno = MKDEV(DEVICE_MAJOR, DEVICE_MINOR);
        mycdev = cdev_alloc();
        cdev_init(mycdev, &s3c2440_fops);
        err = cdev_add(mycdev, devno, 1);
        if (err != 0)
              printk("s3c2440 485 device register failed!\n");
 
        myclass = class_create(THIS_MODULE, "s3c2440_485");
        if(IS_ERR(myclass)) {
        printk("Err: failed in creating class.\n");
              return -1;
}
 
        class_device_create(myclass,NULL, MKDEV(DEVICE_MAJOR,DEVICE_MINOR), NULL, DEVICE_NAME"%d",DEVICE_MINOR);
	
	printk (DEVICE_NAME"\tdevice initialized\n");

	return 0;
}

module_init(_485_init);

#ifdef MODULE
void __exit _485_exit(void)
{
	writel((readl(base_addr) | S3C2410_LCON_IRM),base_addr);	
	writeb(tmpbak, base);
	iounmap(base);
	iounmap(dat_addr);
	iounmap(base_addr);

	
	unsigned long base;
	unsigned char tmp;

	base = ioremap(0x08000114, 0x400);
	tmp = readb(base);
	tmp &= 0xfe;
	writeb(tmp, base);
	tmp = readb(base);
	iounmap(base);
	
	cdev_del(mycdev);
       	class_device_destroy(myclass,devno);
       	class_destroy(myclass);	

}

module_exit(_485_exit);
#endif

MODULE_LICENSE("GPL");
