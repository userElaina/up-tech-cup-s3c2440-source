/************************************************
 *  GPRS demo, use ppp to connect internet
 *  use ttyS1 to ctrol GPRS
 *  by Zou jian guo <ah_zou@163.com>   
 *  2004-11-02
 *
*************************************************/


#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

//#include <sys/signal.h>
#include <pthread.h>
#include "tty.h"
#include "gprs.h"

/*--------------------------------------------------------*/
#define ENDMINITERM 27 /* ESC to quit miniterm */
#define FALSE		0
#define TRUE  		1
/*--------------------------------------------------------*/
volatile int STOP=FALSE;
int GET_GPRS_OK=FALSE;
int baud=B9600;
int get_baudrate(int argc,char** argv);

char * cmd[20]={
		"AT",
		"ATE1",
		"AT+CHFA=1", 			//设置通话通道为1,AT+CHFA 命令切换主副音频通道
		"AT+CLVL=100",			//设置受话器音量最大, AT+CLVL 命令可以调节输出音频信号增益
		"AT+CMIC=1,10"			//设置通道1的话筒增益
//		"ATD12345678\r",
};


void * keyshell() ;
/*--------------------------------------------------------*/

/*--------------------------------------------------------*/
/* modem input handler */
/*void* gprs_show(void * data)
{
	while(1){
		if(GET_GPRS_OK){
			GET_GPRS_OK=FALSE;
//			printf("%s",GPRS_BUF);
//			GPRS_parse(GPRS_BUF,&gps_info);
//			GPRS_show();
		}
		usleep(100);
		if(STOP)break;
	}

}*/

/*--------------------------------------------------------*/
/* modem input handler */
void* gprs_read(void * data)
{
	int i=0;
	char c;
	char buf[1024];
  	printf("\nread modem\n");
	
  	while (STOP==FALSE) 	{		
		
	    	tty_read(&c,1); 	
		printf("tty_read \n");
		printf("%c",c);	
	}
  	printf("exit from reading modem\n");
  	return NULL; 
}

/*--------------------------------------------------------*/
int main(int argc,char** argv)
{
	int ok;
 	pthread_t th_a, th_b, th_show;
 	void * retval;
	
	
	if(argc < 2) {
     	printf ("  Default baudrate is 9600 bps. If not, please enter the baudrate as a parameter\n");
	}
       else
		baud=get_baudrate(argc, argv);
		tty_init();

  	pthread_create(&th_a, NULL, keyshell, 0);
  	pthread_create(&th_b, NULL, gprs_read, 0);
//  	pthread_create(&th_show, NULL, gprs_show, 0);

	
	while(!STOP){
		usleep(100000);
	}
  	tty_end();
  	exit(0); 
}

int get_baudrate(int argc,char** argv)
{
	int v=atoi(argv[1]);
	 switch(v){
		case 4800:
			return B4800;
			
		case 9600:
			return B9600;
			
		case 19200:
			return B19200;
			
		case 38400:
			return B38400;
			
		case 57600:
			return B57600;
			
		case 115200:
			return B115200;
		default:
			return -1;
	 } 	 
}

