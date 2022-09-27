/************************************************
 *  TTY SERIAL ROUTES 
 *  use ttyS1 to ctrol GPRS
 *  by Zou jian guo <ah_zou@163.com>   
 *  2004-11-02
 *  
 *  edited by wbin <wbinbuaa@163.com>
 *  2005-01-19
 *
 *  edited by lyj_uptech@126.com
 *  2007-3-11
 *  目前仍然存在的问题是清空串口的缓冲区//解决2007-3-22
 *
 *  
*************************************************/
//#include <termios.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/signal.h>
//#include <pthread.h>
//#include "tty.h"

#include "tty.h"
#define BAUDRATE B57600//设置条码扫描仪的串口通讯波特率为57600BPS
//#define COM2 "/dev/ttySAC1"//应用于串口2与BarCode Scanner通讯
//#define COM2 "/dev/tts/1"//应用于串口2与BarCode Scanner通讯
#define COM2 "/dev/ttySAC2"
static int fd;
static struct termios oldtio,newtio,oldstdtio,newstdtio;

//==============================================================
int tty_end()
{
  	tcsetattr(fd,TCSANOW,&oldtio); /* restore old modem setings */
  //	tcsetattr(0,TCSANOW,&oldstdtio); /* restore old tty setings */
  	close(fd);
  	printf("\nset COM1 as before!!!\n");
	close(fd);
}
//==============================================================
/*static void do_exit()
{
	tty_end();
	exit(1);
}*/
//==============================================================
int tty_read(char *buf,int nbytes)
{
	int i,status = 0;
	status = read(fd,buf,nbytes);
//	printf("\nwe read:");
	if(status>=0)
//		for(i=0;i<nbytes;i++)
//			printf(" %x",buf[i]);
	return status;
}
//==============================================================
int tty_write(char *buf,int nbytes)
{
	int i,code = 0;
//	printf("\nwe write: ");
	for(i=0; i<nbytes; i++) {
		code += write(fd,&buf[i],1);
//		printf(" %x",buf[i]);
		usleep(1000);
	}
//	printf("\n we totle write %d",code);
	return tcdrain(fd);
}

//==============================================================
int tty_writecmd(char *buf,int nbytes)
{
	int i,code=0;
	for(i=0; i<nbytes; i++) {
		code += write(fd,&buf[i],1);
		usleep(100);
	}
//	printf("\n we totle write %d",code);
//	write(fd,"\r",1);
	usleep(300000);
	return tcdrain(fd);
}

//==============================================================
int tty_writeByte(unsigned char ch)
{
	int status = 0;
	status = write(fd,&ch,1);
//	usleep(10);
	return status;
}
//=============================================================
unsigned char  tty_readByte()
{
	unsigned char ch;
	int status = 0;
	status = read(fd,&ch,1);
//	usleep(10);
	return ch;
}
//==============================================================
int tty_init()
{	int status =0;
	printf("\n *****try to open com1*****");
	fd = open(COM2, O_RDWR ); //| O_NONBLOCK);//
	if(fd<0) {
	    	perror(COM2);
			printf("\n open failed");
	    	exit(-1);
  	}
	printf("\n *****open com1 success*****");
	
  	tcgetattr(0,&oldstdtio);
  	tcgetattr(fd,&oldtio); /* save current modem settings */
	//查看当前设置
/**	
  printf("\n old stdinputspeed: %d",cfgetispeed(&oldstdtio));
	printf("\n old stdoutputspeed: %d",cfgetospeed(&oldstdtio));
	printf("\n old inputspeed: %d",cfgetispeed(&oldtio));
	printf("\n old outputspeed: %d",cfgetospeed(&oldtio));
*/
  tcgetattr(fd,&newstdtio); /* get working stdtio */
	tcflush(fd, TCIOFLUSH); //刷新输入输出缓冲
	cfsetispeed(&newtio, BAUDRATE); //这里分别设置 
	cfsetospeed(&newtio, BAUDRATE); 
	status = tcsetattr(fd, TCSANOW, &newtio); //这是立刻把bote rates设置真正写到串口中去 
	if(status!= 0){
		printf("\ntcsetattr set baudrate faild!!!");
		perror("tcsetattr fd1"); //设置错误
		return 1;
	}
	//设置8，1，n 注意，这里和上面要和下位机相符才可能通信
  if(set_Parity(fd,8,1,'N')==0){
		printf("\nSet Parity Error\n");
		return 1;
	}
 /* now clean the modem line and activate the settings for modem */
//	printf("\n new inputspeed: %d",cfgetispeed(&newtio));
//	printf("\n new outputspeed: %d",cfgetospeed(&newtio));
	printf("\n ********set COM1 succeed! **********");
	return 0;
}
/******************************************************
* @auther lyj_uptech@126.com
* 2007-3-11
******************************************************/
//新加子函数，设置数据位，停止位，校验位
int set_Parity(int fd,int databits,int stopbits,int parity) 
{       int FALSE=0,TRUE=1;
		struct termios options; //定义一个结构
		if( tcgetattr( fd,&options)!=0) //首先读取系统默认设置options中,必须
		{
			perror("SetupSerial 1");
			return(FALSE);
		}
		options.c_cflag &= ~CSIZE; //这是设置c_cflag选项不按位数据位掩码
		switch (databits) /*设置数据位数*/ 
		{
			case 7:
			options.c_cflag |= CS7; //设置c_cflag选项数据位为7位
			break;
			case 8:
			options.c_cflag |= CS8; //设置c_cflag选项数据位为8位
			break;
			default:
			fprintf(stderr,"Unsupported data size\n"); //其他的都不支持
			return (FALSE);
		}
		switch (parity) //设置奇偶校验，c_cflag和c_iflag有效
		{ 
			case 'n':
			case 'N': //无校验当然都不选
			options.c_cflag &= ~PARENB; /* Clear parity enable */
			options.c_iflag &= ~INPCK; /* Enable parity checking */
			break;
			case 'o': //奇校验 其中PARENB校验位有效；PARODD奇校验 INPCK检查校验

			case 'O': options.c_cflag |= (PARODD | PARENB);/* 设置为奇效验*/ 
			options.c_iflag |= INPCK; /* Disnable parity checking */
			break;
			case 'e':
			case 'E': //偶校验，奇校验不选就是偶校验了
			options.c_cflag |= PARENB; /* Enable parity */
			options.c_cflag &= ~PARODD; /* 转换为偶效验*/
			options.c_iflag |= INPCK; /* Disnable parity checking */
			break;
			default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
		}
		/* 设置停止位*/ 
		switch (stopbits) //这是设置停止位数，影响的标志是c_cflag
		{
			case 1:
			options.c_cflag &= ~CSTOPB; //不指明表示一位停止位
			break;
			case 2:
			options.c_cflag |= CSTOPB; //指明CSTOPB表示两位，只有两种可能
			break;
			default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
		}
		/* Set input parity option */
		if (parity != 'n') //这是设置输入是否进行校验
		options.c_iflag |= INPCK;
		
		//这个地方是用来设置控制字符和超时参数的，一般默认即可。稍微要注意的是c_cc数组的VSTART 和 VSTOP 元素被设定成DC1 和 DC3，代表ASCII 标准的XON和XOFF字符。所以如果在传输这两个字符的时候就传不过去，这时需要把软件流控制屏蔽 options.c_iflag &= ~(IXON | IXOFF | IXANY);

		options.c_cc[VTIME] = 150; // 15 seconds 
		options.c_cc[VMIN] = 0;

		tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */ //刷新和立刻写进去
		if (tcsetattr(fd,TCSANOW,&options) != 0) 
		{
			perror("SetupSerial 2");
			return (FALSE);
		}
		return (TRUE);

}
/**
**
**刷新输入输出缓存
** @auther lyj_uptech@126.com
** @date 2007-3-22
**/
void tty_fflush(){
	tcflush(fd,TCIFLUSH);//刷新输入输出缓冲
}

