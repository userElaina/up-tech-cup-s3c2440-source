#include "consoleshell.h"
#undef uchar
#define uchar unsigned char
char shell_s[]="\nshell> ";
/**
*
*/
void get_line(char * cmd);
void * consoleshell()
{
	int i;
	char cmd[256]={0,};	
	uchar addr[2]={0x00,0x00};	
	printf("\n<zigbee ctrl remote control shell>");
	printf("\n [1]  open 	led1");
	printf("\n [2]  close 	led1");
	printf("\n [3]  open 	led2");
	printf("\n [4]  close 	led2");
	printf("\n [**] help 	menu");
	printf("\n [0]  exit");
	while(1){
		printf(shell_s);
		fflush(stdout);
		get_line(cmd);
		printf("\r\nwe get cmd %s",cmd);				//显示必要的输出
		if(strncmp("1",cmd,1)==0){
			tty_fflush();
			zigbee_set_LED(addr,0xA2,0x01);
			fflush(stdout);
		}else if(strncmp("2",cmd,1)==0){
			tty_fflush();
			zigbee_set_LED(addr,0xA2,0x02);
			fflush(stdout);
		}else if(strncmp("3",cmd,1)==0){
			tty_fflush();
			zigbee_set_LED(addr,0xA3,0x01);
			fflush(stdout);
		}else if(strncmp("4",cmd,1)==0){
			tty_fflush();
			zigbee_set_LED(addr,0xA3,0x02);
			fflush(stdout);
		}else if(strncmp("5",cmd,1)==0){
			tty_fflush();
			zigbee_set_default();
			printf("\ncmd 5\n");
		}else if (strncmp("**",cmd,2)==0){
        		printf("\n<zigbee ctrl remote control shell>");
        		printf("\n [1]  open 	led1");
        		printf("\n [2]  close 	led1");
        		printf("\n [3]  open 	led2");
        		printf("\n [4]  close 	led2");
        		printf("\n [**] help 	menu");
        		printf("\n [0]  exit");

		}else if (strncmp("0",cmd,1)==0){
			exit(0);
		}
	}		
}

void get_line(char * cmd)
{
	int i=0;
	while(1){
		cmd[i]=getchar();
		if(cmd[i]==10){
			cmd[i]=0;
			break;
		}
		fflush(stdout);
		i++;
	}

}
void get_words(char *data){
	int i=0;
	while(1){
		data[i]=getchar();
		if(data[i]==10){
			data[i]=0;
			break;
		}
		fflush(stdout);
	}
}

