#include "consoleshell.h"
#undef uchar
#define uchar unsigned char
char shell_s[]="\nshell> ";
uchar command = 0;
void get_line(char *cmd)
{
	int i=0;
	uchar cmd_tmp;
	while(1){
		cmd[i]=getchar();
		if(cmd[i]==10){
                        cmd[i]=0;
                        break;
                }
		//printf(" 0x%x",cmd[i]);
		if(cmd[i] >= '0' && cmd[i] <= '9'){
			cmd_tmp = cmd[i] & 0x0f;
		}else if(cmd[i] >= 'a' && cmd[i] <= 'f'){
			cmd_tmp = 0x09 + cmd[i]&0x0f;	
		}
		cmd[i] = cmd_tmp;
		//printf(" 0x%x",cmd[i]);
		fflush(stdout);
		i++;
	}

}

/**
*
*/
void * consoleshell()
{
	int i;
	char cmd[256]={0,};	
	
	printf("\n<sensor_board control shell>");
	printf("\n  [10] ~ [20] enter command");
	printf("\n  [00] exit");
	while(1){
		printf(shell_s);
		command = 0;
		fflush(stdout);
		get_line(cmd);
		command = (cmd[0] << 4) | cmd[1];
		if(command == 0)
			break;
		if(command > 0x20 || command < 0x10)
		{	
			printf("unknown command!\n");	
			continue;
		}
		printf("we get cmd:0x%x\n",command);
		fflush(stdout);
		sensor(command,0);	
		fflush(stdout);
	};
}

