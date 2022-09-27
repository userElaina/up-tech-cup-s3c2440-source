/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/


/***************************************************************************\
    #˵��: ����̨����

	ͨ�����пڽ����û������Ȼ�󽫽�����͵����ڡ�
	��Ҫ�ṹ����Ҫ����������������Ϳ��Ƴ�������ֲ��
	��������仯�ܴ�

��������
	//fix me!!
	��commands���������һ���һ������Ϊ�����ַ������ڶ�������Ϊ�ص�������
	�ص���������Ϊint callback(int argc, char *argv[])��
	����������dos�µ�main�����Ķ�����ͬ��һ�����������ַ�����Ŀ��һ�����������ַ���ָ��
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	1��������书��
	----------------------------------����--------------------------------------
	2003-6-11	1������

\***************************************************************************/
#include "../inc/command/console.h"
#include <string.h>
#include <stdio.h>

#include "cmddef.c"

void ShowShellMenu(void)
{
	int i,j;
	
	printf("*--------------------------Shell Menu-----------------------------------*\n");

	for(i=0;i<SYSFUNCNUM;i++){
		if(sysfunc[i]->pShellMenu==NULL)
			continue;

		printf("*                %c >>%s",sysfunc[i]->MenuAcc,sysfunc[i]->pShellMenu);

		for(j=51-strlen(sysfunc[i]->pShellMenu);j>0;j--)	//����ո�
			putchar(' ');

		printf("*\n");
	}

	printf("*-----------------------------------------------------------------------*\n");
}

void Shell_input(void)
{
	int i;
	for(;;)
	{
		char aa;
		ShowShellMenu();
		aa=getchar();

		for(i=0;i<SYSFUNCNUM;i++){
			if(sysfunc[i]->MenuAcc == aa)
				sysfunc[i]->func(1, NULL);
		}
	}
}

int Console_input(int argc, char *argv[])
{
	static char commandline[MAX_COMMANDLINE_LENGTH];
	int cmmandlen;
	
	printf("Type \"help\" to list all command.\n");
	
	for(;;)
	{
		printf("\n%s>", currentpath);
		cmmandlen=GetCommand(commandline,MAX_COMMANDLINE_LENGTH);
		commandline[cmmandlen]=0;
		if(strncmp(commandline, "exit", 4) == 0)
			break;
		if(cmmandlen > 0)
			if(parse_command(commandline)==-ECOMMAND)
				printf("Can't find this Command!\n");
	}

	return OK;
}

void showhelp(int n)
{
	char* p=sysfunc[n]->pHelp;

	putchar('\n');

	if(p==NULL)
		return;

	while(*p){
		putchar(*p);
		p++;
	}
}

//�����з���
#define STATE_WHITESPACE (0)
#define STATE_WORD (1)
void parse_args(char *cmdline, int *argc, char **argv)
{
	char *c;
	int state = STATE_WHITESPACE;
	int i;

	*argc = 0;

	if(strlen(cmdline) == 0)
		return;
	c = cmdline;
	while(*c != '\0') {
		if(*c == '\t')
			*c = ' ';

		c++;
	}
	
	c = cmdline;
	i = 0;

	while(*c != '\0') {
		if(state == STATE_WHITESPACE) {
			if(*c != ' ') {
				argv[i] = c;
				i++;
				state = STATE_WORD;
			}
		} else {
			if(*c == ' ') {
				*c = '\0';
				state = STATE_WHITESPACE;
			}
		}

		c++;
	}
	
	*argc = i;
}

//���Ҷ�Ӧ����ṹ�е������ַ����ĸ���
int get_num_command_matches(char *cmdline)
{
	int i,j;
	int num_matches = 0;
	j=SYSFUNCNUM;	//(sizeof(commands)/sizeof(commandlist_t));
	for(i=0;i<j;i++) {
		if(strcmp(sysfunc[i]->pCommand, cmdline) == 0) 
			num_matches++;
	}

	return num_matches;
}

//�����û����룬֮����Ӧ�ص�����
int parse_command(char *cmdline)
{
	static char *argv[MAX_ARGS];
	int argc, num_commands;
	int i;

	memset(argv, 0, sizeof(argv));
	parse_args(cmdline, &argc, argv);
	
	if(argc == 0) 
		return 0;
	num_commands = get_num_command_matches(argv[0]);

	if(num_commands < 0)
		return num_commands;
	
	if(num_commands == 0)
		return -ECOMMAND;
	
	if(num_commands > 1)
		return -EAMBIGCMD;

	for(i=0;i<SYSFUNCNUM;i++) {
		if(strcmp(sysfunc[i]->pCommand, cmdline) == 0) {
			if(strcmp("-help", argv[1]) == 0) {
				showhelp(i);
				return OK;
			}
			else
				return sysfunc[i]->func(argc, argv);
		}
	}

	return -ECOMMAND;
}

//�Ӵ��ڻ�ȡ�û����룬������
int GetCommand(char *command, int len)
{
	unsigned char c;
	int i;
	int numRead;
	int maxRead = len - 1;

	for(numRead = 0, i = 0; numRead < maxRead;) {
		/* try to get a byte from the serial port */
		c=getchar();

		if((c == '\r') || (c == '\n')) {
			command[i++] = '\0';
			putchar(c);

			/* print newline */
			return(numRead);
		} else if(c == '\b') { /* FIXME: is this backspace? */
			if(i > 0) {
				i--;
				numRead--;
				/* cursor one position back. */
				printf("\b \b");
			}
		}else {
			command[i++] = c;
			numRead++;

			/* print character */
			putchar(c);
		}
	}
	return(numRead);
}


//�����

//�����û�������
int echo_callback(int argc, char *argv[])
{
	if(argc>1)
	{
		printf("\n");
		printf(argv[1]);
	}
	return OK;
}

int help_callback(int argc, char *argv[])
{
	int i;
	if(argc<1)
	{
		printf("\nParameter eror!");
		return FAIL;
	}

	printf("*-----------------------------------------------------------------------*\n");
	for(i=0;i<SYSFUNCNUM;i++) {
		if(sysfunc[i]->pCommand)
			printf("%s\n",sysfunc[i]->pCommand);
	}
	printf("*-----------------------------------------------------------------------*\n");
	printf("Type -help follow command for more information.\n");
	return OK;
}

#if 0

void xdl_helpcallback(void)
{
	printf("\n");
	printf("\nUsage: xdl [FILE] [OPTION]");
	printf("\n");
	printf("\n  Download file from host via xmodem protocol.");
	printf("\n");
	printf("\nFILE    :");
	printf("\n  filename   the file name you want to save to the flash");
	printf("\n  -help      display this help and exit");
	printf("\nOPTION  :");
	printf("\n  -t         this is a text file");
	printf("\n  -d         this is a data file");
	printf("\n");
}

//�ϴ��ļ�
int xul_callback(int argc, char *argv[])
{
	//int len;
	if(argc<3)
	{
		printf("\nParameter eror!");
		return FALSE;
	}
	
	if(strncmp("-d",argv[2],2)==0)
	{
		printf("\nTransmit CNCFILE in data CNCFILE mode.");
		return XTransmitFile(argv[1],XMODEM_DAT);
	}
	if(strncmp("-t",argv[2],2)==0)
	{
		printf("\nTransmit CNCFILE in txt CNCFILE mode.");
		return XTransmitFile(argv[1],XMODEM_TXT);
	}
         return FALSE;
}

void xul_helpcallback(void)
{
}

//ɾ���ļ�
int xdel_callback(int argc, char *argv[])
{
	//int len;
	if(argc<2)
	{
		printf("\nParameter eror!");
		return FALSE;
	}
	DeleteOSFile(nCurFSpartition, argv[1]);
         return TRUE;
}

void xdel_helpcallback(void)
{
}

//�����ļ�
int xcp_callback(int argc, char *argv[])
{
	//int i;
	if(argc<3)
	{
		printf("\nParameter eror!");
		return FALSE;
	}
	CopyOSFile(argv[1],argv[2], nCurFSpartition, nCurFSpartition);
         return TRUE;
}

void xcp_helpcallback(void)
{
}

//�ļ�����
int xrn_callback(int argc, char *argv[])
{
	//int i;
	if(argc<3)
	{
		printf("\nParameter eror!");
		return FALSE;
	}
	RenameOSFile(nCurFSpartition, argv[1],argv[2]);
         return TRUE;
}

void xrn_helpcallback(void)
{
}


//�洢����ʽ��
int xfm_callback(int argc, char *argv[])
{
	unsigned char aa;
	if(argc<1)
	{
		printf("\nParameter eror!");
		return FALSE;
	}
	printf("\nFormat Will DESTROY ALL DATA!A YOU SURE?(N)");
	OSReadUart0(&aa,1,50000);
	Uart_SendByte(0,aa);
	if (aa == 'Y')
	{
		FormatFS(nCurFSpartition);
	}
	else
	{
		printf("\nCanceled.");
	}
         return TRUE;
}

void xfm_helpcallback(void)
{
}

//��ʾ�ļ�
int xtp_callback(int argc, char *argv[])
{
	//int len;

	CNCFILE *file1;
	int i;
	U8 fchar;
	
	if(argc<2)
	{
		printf("\nParameter eror!");
		return FALSE;
	}
	
	file1=OpenOSFile(nCurFSpartition, argv[1],FILEMODE_READ);
	if(file1==NULL)
	{
		printf("\nNO such CNCFILE!");
		return FALSE;
	}
	printf("\n");
	for(i=0;i<file1->filesize;i++)
	{
		ReadOSFile(file1,&fchar,1);
		Uart_SendByte(0,fchar);
	}
	printf("\nFile length is %d bytes.",file1->filesize);
	CloseOSFile(file1);
         return TRUE;
	
}

void xtp_helpcallback(void)
{
}

//LCDģʽ�л�
int xchlm_callback(int argc, char *argv[])
{
	if(LCDdspMode==DspGraMode)
	{
		LCD_ChangeMode(DspTxtMode);
		LCD_printf("");
	}
	else
	{
		LCD_ChangeMode(DspGraMode);
		OSMboxPost(LCDFresh_MBox,(void*)1);	//ˢ��LCD
		OSMboxPost(LCDFresh_MBox,(void*)1);	//ˢ��LCD
	}
         return TRUE;
}

void xchlm_helpcallback(void)
{
}

//�л�Ŀ¼
int xcd_callback(int argc, char *argv[])
{
	int i;
	if(argc !=2 )
	{
		printf("\nParameter error!");
		return FALSE;
	}

	for(i=0;i<FsPart_Num;i++){
		if(strcmp(FsPart[i]->PartitionName, argv[1])==0){
			nCurFSpartition=i;
			return TRUE;
		}
	}

	printf("\nCan not find File System!\tname=%s!",argv[1]);
	return FALSE;
}

void xcd_helpcallback(void)
{
}

//
int xdf_callback(int argc, char *argv[])
{
	int i;
	if(argc<1)
	{
		printf("\nParameter eror!");
		return FALSE;
	}

	printf("\nPartition Name  BlockSize(byte)  TotalBlocks  Used Blocks-Percent  bad Blocks");
	for(i=0;i<FsPart_Num;i++){
		int usedblk,totalblk, unusedblk,badblk,usedpercent,blksize;
		blksize=GetFS_Info(i, &totalblk, &unusedblk, &badblk);
		usedblk=totalblk-unusedblk;
		usedpercent=usedblk*100/totalblk;

		printf("\n%14s%17d%13d%13d%7d%%%12d", FsPart[i]->PartitionName, 
				blksize, totalblk, usedblk, usedpercent, badblk);

	}
	printf("\n");
  	return TRUE;
}
#endif

//
