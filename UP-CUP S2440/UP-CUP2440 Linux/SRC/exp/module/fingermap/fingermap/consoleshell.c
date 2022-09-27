#include "consoleshell.h"
char shell_s[]="\nshell> ";
void get_line(char *);
/**
**
**控制台
**
*/
void consoleshell(){
	int count=0;
	int i;
	char cmd[256]={0,};	
	int  num;
//	fingermap_init();
	printf("\nfingermap inited !");
	printf("\n<fingermap control shell>");
	printf("\n [1]  Get fingermap record num");
	printf("\n [2]  User Enroll");
	printf("\n [3]  User Auth");
	printf("\n [4]  Delete fingermap record");
	printf("\n [5]  Clear all fingermap record");
	printf("\n [0]  Print the end of the context an exit");
	printf("\n [**] help menu");
	printf("\n [0]  print the end of the context an exit");
	while(1){
		printf(shell_s);
		fflush(stdout);
		get_line(cmd);
//		printf("\r\nwe get it!!!!!!!!!!!!");				//显示必要的输出

		if(strncmp("1",cmd,1)==0){
			printf("\nget num\n");
			Get_FPNum();
		}else if(strncmp("2",cmd,1)==0){
			fflush(stdin);//刷新缓冲区
		  	tty_fflush();//2007-3-23刷新缓存
			printf("Please input user's fingerprint!\n");
			User_enroll(0xff);
		}else if(strncmp("3",cmd,1)==0){
			printf("Please input user's fingerprint!\n");
			tty_fflush();
			User_auth(0xff);
		}else if(strncmp("4",cmd,1)==0){
			tty_fflush();
			User_erase(num);
		}else if(strncmp("5",cmd,1)==0){
			printf("clear all store\n");
			tty_fflush();
			User_clear();
		}else if(strncmp("**",cmd,2)==0){
			printf("\n<fingermap control shell>");
			printf("\n [1]  Get fingermap record num");
			printf("\n [2]  User Enroll");
			printf("\n [3]  User Auth");
			printf("\n [4]  Delete fingermap record");
			printf("\n [5]  Clear all fingermap record");
			printf("\n [0]  Print the end of the context an exit");
			printf("\n [**] help menu");
			printf("\n [0]  print the end of the context an exit");
		}else if(strncmp("0",cmd,1)==0){
				char ch[1];
/**				for(;;){
					tty_read(ch,1);
					printf("%c",ch[0]);
				}
*/				break;
		}else if(cmd[0] != "0"){
			 system(cmd);
		}
	}
}

void get_line(char *cmd){
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
/**
void fingermap_init(){
	Set_config();//初始化模块配置
	//验证管理员指纹
	//如果为管理员指纹则可以进行所有操作，如果是普通用户则只能进行验证操作
}
*/

