/**
 * s3c2440-da.c
 *
 * da driver for UP-CUP2440 DA
 *
 * Author: Wang bin <wbinbuaa@163.com>
 * Date  : $Date: 2005/7/22 15:50:00 $ 
 * $Revision: 2.1.0 $
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * History:
 *
 * 
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <asm/uaccess.h>				/* copy_from_user */

#include <asm/arch/spi.h>
#include <asm/io.h>
#include <asm/hardware.h>
// by sprife
#include <asm/plat-s3c24xx/regs-spi.h>
#include <asm-arm/arch-s3c2410/regs-gpio.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h> 
#include <asm/arch/hardware.h>
#include <asm/irq.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>


//add end
/* debug macros */
#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK(x...) {printk(__FUNCTION__"(%d): ",__LINE__);printk(##x);}
#else
#define DPRINTK(x...) (void)(0)
#endif

/* debug macros */
#define STEP_DEBUG
#ifdef STEP_DEBUG
#define DPRINTK_STEP(x...) {printk("%s: ", __FUNCTION__);printk(x);}
#else
#define DPRINTK_STEP(x...) (void)(0)
#endif

#define DEVICE_NAME	"s3c2440-da"
#define EXIORAW_MINOR	1
#define DA0_IOCTRL_WRITE 			0x10
#define DA1_IOCTRL_WRITE			0x11
#define DA_IOCTRL_CLR 				0x12
#define STEPMOTOR_IOCTRL_PHASE 		0x13
//
#define GPIO_TC_SPIMISO         (GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E11)
#define GPIO_TC_SPIMOSI         (GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E12)
#define GPIO_TC_SPICLK          (GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E13)

static int exioMajor = 0;

static struct spi_device *da_spi;
signed int da_addr;
static void  spi_send(unsigned char val/*, int channel*/);
#define DA_FLAG		(1<<3)  
struct class_simple *myclass;
dev_t dev=0;

/*************************************************************************/
static int s3c2440_exio_open(struct inode *inode, struct file *filp)
{
	DPRINTK( "S3c2440 DA device open!\n");
	return 0;
}

static int s3c2440_exio_release(struct inode *inode, struct file *filp)
{
	DPRINTK( "S3c2440 DA device release!\n");
	return 0;
}

/*******Enable the select port of DA ********/

static ssize_t da_enable(size_t ndev )
{
	writel((readl(da_addr) & (~DA_FLAG)),da_addr);
	udelay(1000);
	return 0;	
}
/*******Disable the select port of DA ********/
static ssize_t da_disable(size_t ndev )
{
	writel((readl(da_addr) | DA_FLAG),da_addr);
	udelay(1000);
	return 0;	
}

/*******Write the select port of DA ********/
static ssize_t da_write(size_t ndev, const char *buffer)
{
	unsigned int value;
	char buf[4];

	copy_from_user(buf, buffer, 4);
	value = *((int *)buf);	
	value <<= 2;

	da_enable(ndev); // by sprife ";"	
  	spi_send((value>>8) &0xff/*, 0*/);
  	spi_send((value&0xff)/*, 0*/);
	da_disable(ndev);

	DPRINTK("write to max504-1 => %u\n",value);
	return 0;
}
/*******Clear the select port of DA ********/
static ssize_t da_clear(int ndev)
{
        da_disable(ndev);
	udelay(1000);
	return 0;	
}
static int s3c2440_exio_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd){
	
	/*********write da 0 with (*arg) ************/		
	case DA0_IOCTRL_WRITE:
		return da_write(0, (const char *)arg);

	/*********write da 1 with (*arg) ************/		
	case DA1_IOCTRL_WRITE:
		return da_write(1, (const char *)arg);

	/*********clear da (*arg) *****************/		
	case DA_IOCTRL_CLR:
		return da_clear((int)arg);
	}

	return 0;	
}

static struct file_operations s3c2440_exio_fops = {
	owner:	THIS_MODULE,
	open:	s3c2440_exio_open,
	ioctl:	s3c2440_exio_ioctl,
	release:  s3c2440_exio_release,
};

static int test_read_reg(struct spi_device *spi, int reg)
{
	char buf[2];
	buf[0] = reg << 2;
	buf[1] = 0;
	spi_write_then_read(spi, buf, 2, buf, 2);
	return buf[1] << 8 | buf[0];
} 

static void  spi_send(unsigned char val/*, int channel*/){
	char txbuf[1];
	int ret;
	txbuf[0] = val;
	/* write in one transfer to avoid data inconsistency */
	ret = spi_write_then_read(da_spi, txbuf, sizeof(txbuf), NULL, 0);
	mdelay(400);//only for 2440
	if(ret)
		printk("spi send err!");
}

static int uptech_spi_probe(struct spi_device *spi)
{
        /* We'd set TX wordsize 8 bits and RX wordsize to 13 bits ... except
         * that even if the hardware can do that, the SPI controller driver
         * may not.  So we stick to very-portable 8 bit words, both RX and TX.
         */

        spi->bits_per_word = 8;
        spi->dev.power.power_state = PMSG_ON;

	da_spi = spi;
	//printk("TEST_REG: 0x%02x\n", test_read_reg(spi, TEST_REG));
	return 0;
}

static int uptech_spi_remove(struct spi_device *spi)
{
	return 0;
}

static struct spi_driver uptech_spi_driver = {
	.probe = uptech_spi_probe,
	.remove = uptech_spi_remove,
	.driver = {
	.name = "uptech_spi",
        //.bus    = &spi_bus_type,
        //.owner  = THIS_MODULE,
	},
};
int __init s3c2440_exio_init(void)
{
	int ret;
	da_addr = ioremap(0x56000064,0x20);

	s3c2410_gpio_cfgpin(S3C2410_GPG3,S3C2410_GPG3_OUTP);  
	s3c2410_gpio_pullup(S3C2410_GPG3, 0);   
	
	ret = register_chrdev(0, DEVICE_NAME, &s3c2440_exio_fops);
	if (ret < 0) {
		DPRINTK(DEVICE_NAME " can't get major number\n");
		return ret;
	}
	exioMajor=ret;
	dev=MKDEV(exioMajor,0);
        myclass = class_create(THIS_MODULE,"da");
        class_device_create(myclass,NULL,dev,NULL,"da");
	
	ret = spi_register_driver(&uptech_spi_driver); 
	if (ret < 0) {
		DPRINTK(DEVICE_NAME " spi driver register err\n");
		return ret;
	}


	printk(DEVICE_NAME"\tdevice initialized\n");
	return 0;
	
}

module_init(s3c2440_exio_init);

#ifdef MODULE
void __exit s3c2440_exio_exit(void)
{
	iounmap(da_addr);
	 class_device_destroy(myclass,dev);
        class_destroy(myclass);

	unregister_chrdev(exioMajor, DEVICE_NAME);
	spi_unregister_driver(&uptech_spi_driver);
}

module_exit(s3c2440_exio_exit);
#endif
MODULE_LICENSE("GPL"); 

