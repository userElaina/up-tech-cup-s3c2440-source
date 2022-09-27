
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
//#include <sys/signal.h>
#include "tty.h"
#include"gprs.h"
void gprs_init()
{ 
	int i;
	for(i=0; i<5; i++) {
		tty_writecmd(cmd[i], strlen(cmd[i])); 
	}

	//ÏÂÃæÑ¡ÔñÍ¨µÀ0£¬Ê¹ÓÃÆÕÍ¨µç»°»°±ú
	/*
	  gprs_cmd("AT+CHFA=0\r"); //ÉèÖÃÍ¨»°Í¨µÀÎª0
	  usleep(1000);
	  gprs_cmd("AT+CLVL=100\r");//ÉèÖÃÊÜ»°Æ÷ÒôÁ¿×î´ó
	  usleep(1000);
	  gprs_cmd("AT+CMIC=0,10\r");//ÉèÖÃÍ¨µÀ0µÄ»°Í²ÔöÒæ
	  usleep(1000);
	*/
}

void gprs_hold()
{
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATH", strlen("ATH"));//·¢ËÍ¹Ò»úÃüÁîATH
}

void gprs_ans()
{ 
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATA", strlen("ATA"));//·¢ËÍ½ÓÌýÃüÁîATA
}

//²¦½Ð  
void gprs_call(char *number, int num)			
{ 

	tty_write("ATD", strlen("ATD")); //·¢ËÍ²¦´òÃüÁîATD	
	tty_write(number, num);
	tty_write(";\r", strlen(";\r"));
	usleep(200000);
}

//·¢ËÍ¶ÌÐÅ
//void gprs_msg(char *number,char *text) 			//·¢ËÍ¶ÌÐÅ
void gprs_msg(char *number, int num)		
{ 
	char ctl[]={26,0};
	char text[]="Welcome to use up-tech embedded platform!";
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("AT+CMGF=1", strlen("AT+CMGF=1"));		//·¢ËÍÐÞ¸Ä×Ö·û¼¯ÃüÁî
	tty_write("AT+CMGS=", strlen("AT+CMGS="));	//·¢ËÍ·¢¶ÌÐÅÃüÁî£¬¾ßÌå¸ñÊ½¼ûÊÖ²á

    tty_write("\"", strlen("\""));
    tty_write(number, strlen(number));
    tty_write("\"", strlen("\""));
    tty_write(";\r", strlen(";\r"));
    
	usleep(300000);
    // tty_writecmd(number, strlen(number));
	tty_write(text, strlen(text));
	tty_write(ctl, 1);
	usleep(300000);
//	Uart_SendByte(1,26); //¡°CTRL+Z"µÄASCIIÂë
}

void gprs_baud(char *baud,int num)
{
	tty_write("AT+IPR=", strlen("AT+IPR="));
	tty_writecmd(baud, strlen(baud) );
	usleep(200000);
}
