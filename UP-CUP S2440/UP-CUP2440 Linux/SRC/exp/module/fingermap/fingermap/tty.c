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
 *  Ŀǰ��Ȼ���ڵ���������մ��ڵĻ�����//���2007-3-22
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
#define BAUDRATE B57600//��������ɨ���ǵĴ���ͨѶ������Ϊ57600BPS
//#define COM2 "/dev/ttySAC1"//Ӧ���ڴ���2��BarCode ScannerͨѶ
//#define COM2 "/dev/tts/1"//Ӧ���ڴ���2��BarCode ScannerͨѶ
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
	//�鿴��ǰ����
/**	
  printf("\n old stdinputspeed: %d",cfgetispeed(&oldstdtio));
	printf("\n old stdoutputspeed: %d",cfgetospeed(&oldstdtio));
	printf("\n old inputspeed: %d",cfgetispeed(&oldtio));
	printf("\n old outputspeed: %d",cfgetospeed(&oldtio));
*/
  tcgetattr(fd,&newstdtio); /* get working stdtio */
	tcflush(fd, TCIOFLUSH); //ˢ�������������
	cfsetispeed(&newtio, BAUDRATE); //����ֱ����� 
	cfsetospeed(&newtio, BAUDRATE); 
	status = tcsetattr(fd, TCSANOW, &newtio); //�������̰�bote rates��������д��������ȥ 
	if(status!= 0){
		printf("\ntcsetattr set baudrate faild!!!");
		perror("tcsetattr fd1"); //���ô���
		return 1;
	}
	//����8��1��n ע�⣬���������Ҫ����λ������ſ���ͨ��
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
//�¼��Ӻ�������������λ��ֹͣλ��У��λ
int set_Parity(int fd,int databits,int stopbits,int parity) 
{       int FALSE=0,TRUE=1;
		struct termios options; //����һ���ṹ
		if( tcgetattr( fd,&options)!=0) //���ȶ�ȡϵͳĬ������options��,����
		{
			perror("SetupSerial 1");
			return(FALSE);
		}
		options.c_cflag &= ~CSIZE; //��������c_cflagѡ���λ����λ����
		switch (databits) /*��������λ��*/ 
		{
			case 7:
			options.c_cflag |= CS7; //����c_cflagѡ������λΪ7λ
			break;
			case 8:
			options.c_cflag |= CS8; //����c_cflagѡ������λΪ8λ
			break;
			default:
			fprintf(stderr,"Unsupported data size\n"); //�����Ķ���֧��
			return (FALSE);
		}
		switch (parity) //������żУ�飬c_cflag��c_iflag��Ч
		{ 
			case 'n':
			case 'N': //��У�鵱Ȼ����ѡ
			options.c_cflag &= ~PARENB; /* Clear parity enable */
			options.c_iflag &= ~INPCK; /* Enable parity checking */
			break;
			case 'o': //��У�� ����PARENBУ��λ��Ч��PARODD��У�� INPCK���У��

			case 'O': options.c_cflag |= (PARODD | PARENB);/* ����Ϊ��Ч��*/ 
			options.c_iflag |= INPCK; /* Disnable parity checking */
			break;
			case 'e':
			case 'E': //żУ�飬��У�鲻ѡ����żУ����
			options.c_cflag |= PARENB; /* Enable parity */
			options.c_cflag &= ~PARODD; /* ת��ΪżЧ��*/
			options.c_iflag |= INPCK; /* Disnable parity checking */
			break;
			default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
		}
		/* ����ֹͣλ*/ 
		switch (stopbits) //��������ֹͣλ����Ӱ��ı�־��c_cflag
		{
			case 1:
			options.c_cflag &= ~CSTOPB; //��ָ����ʾһλֹͣλ
			break;
			case 2:
			options.c_cflag |= CSTOPB; //ָ��CSTOPB��ʾ��λ��ֻ�����ֿ���
			break;
			default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
		}
		/* Set input parity option */
		if (parity != 'n') //�������������Ƿ����У��
		options.c_iflag |= INPCK;
		
		//����ط����������ÿ����ַ��ͳ�ʱ�����ģ�һ��Ĭ�ϼ��ɡ���΢Ҫע�����c_cc�����VSTART �� VSTOP Ԫ�ر��趨��DC1 �� DC3������ASCII ��׼��XON��XOFF�ַ�����������ڴ����������ַ���ʱ��ʹ�����ȥ����ʱ��Ҫ��������������� options.c_iflag &= ~(IXON | IXOFF | IXANY);

		options.c_cc[VTIME] = 150; // 15 seconds 
		options.c_cc[VMIN] = 0;

		tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */ //ˢ�º�����д��ȥ
		if (tcsetattr(fd,TCSANOW,&options) != 0) 
		{
			perror("SetupSerial 2");
			return (FALSE);
		}
		return (TRUE);

}
/**
**
**ˢ�������������
** @auther lyj_uptech@126.com
** @date 2007-3-22
**/
void tty_fflush(){
	tcflush(fd,TCIFLUSH);//ˢ�������������
}

