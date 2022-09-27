/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/


/***************************************************************************\
    #说明: 文件系统的命令行处理函数
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-5-2	1、创建

\***************************************************************************/
#include "../inc/drivers.h"
#include "../inc/command/xmodem.h"
#include "../inc/sys/lib.h"
#include <string.h>
#include <stdio.h>

#define MAX_PATHLENGTH		512

char currentpath[MAX_PATHLENGTH]="/sys";
static char filepath[MAX_PATHLENGTH];

/***************************************************************************\
	列出文件
\***************************************************************************/

void FS_ListAll()
{
	file_DIR *d;
	file_dirent *de;
	file_stat s;
	char str[100];
	
	d = opendir(currentpath);
	
	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",currentpath,de->d_name);
			
			lstat(str,&s);
			
			printf("%-15s length %ld ",de->d_name,s.st_size);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  break;
				default: printf("unknown"); break;
			}
			
			printf("\n");		
		}
		
		closedir(d);
	}
	printf("\n");
	
	printf("Free space in %s is %ld KB\n\n",
		currentpath,free_space(currentpath)/1024);

}

int ls_callback(int argc, char *argv[])
{
	//int i;
	if(argc<1){
		printf("Parameter eror!\n");
		return FALSE;
	}
	printf("File list:\n");
	FS_ListAll();	//add by threewater
         return TRUE;
}

//上传文件
int ul_callback(int argc, char *argv[])
{
	//int len;
	if(argc<3)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	sprintf(filepath, "%s/%s", currentpath, argv[1]);
	
	if(strncmp("-d",argv[2],2)==0)
	{
		printf("Transmit FILE in data FILE mode.\n");
		return XTransmitFile(filepath,XMODEM_DAT);
	}
	if(strncmp("-t",argv[2],2)==0)
	{
		printf("Transmit FILE in text FILE mode.\n");
		return XTransmitFile(filepath,XMODEM_TXT);
	}
         return FALSE;
}

//下载文件
int dl_callback(int argc, char *argv[])
{
	//int len;
	if(argc<3)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	sprintf(filepath, "%s/%s", currentpath, argv[1]);
	if(strncmp("-d",argv[2],2)==0)
	{
		printf("Receive FILE in data FILE mode.\n");
		return XReceiveFile(filepath,XMODEM_DAT);
	}
	if(strncmp("-t",argv[2],2)==0)
	{
		printf("Receive CNCFILE in txt CNCFILE mode.\n");
		return XReceiveFile(filepath,XMODEM_TXT);
	}
   	return FALSE;
}

int FormatNandFlash(int argc, char *argv[])
{
	printf("\nWaring: You will lost all of data in Part %s!\n", currentpath);
	printf("Are you sure?[y/n]\n");

	if(getchar()=='y'){
		//	Format_Fat12Media();
		format(currentpath);
	}

	return OK;
}

//显示文件
int cat_callback(int argc, char *argv[])
{
	//int len;

	FILE *file;
	char fchar;
	
	if(argc<2)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}
	
	sprintf(filepath, "%s/%s", currentpath, argv[1]);

	file=fopen(filepath,"r");
	if(file==NULL)
	{
		printf("NO such FILE!\n");
		return FALSE;
	}
	printf("\n");

	while(fread(&fchar, 1,1, file))
		putchar(fchar);
	
	fclose(file);
         return TRUE;
	
}

//cd 
int cd_callback(int argc, char *argv[])
{
	file_stat s;
	
	if(argc<2)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	if(argv[1][0]!='/')
		sprintf(filepath, "%s/%s", currentpath, argv[1]);
	else
		strcpy(filepath, argv[1]);

	lstat(filepath, &s);
	if((s.st_mode & S_IFDIR)==0){
		printf("Not directory: %s\n", filepath);
		return FALSE;
	}

	strcpy(currentpath, filepath);

	return TRUE;
}

//delete
int rm_callback(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	sprintf(filepath, "%s/%s", currentpath, argv[1]);

	if(unlink(filepath)<0){
		printf("Failed to remove: %s\n", filepath);
		return FALSE;
	}

	printf("remove: %s\n", filepath);
	return TRUE;
}

//rename / move
int mv_callback(int argc, char *argv[])
{
	static char newfilepath[MAX_PATHLENGTH];
	
	if(argc<3)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	sprintf(filepath, "%s/%s", currentpath, argv[1]);
	sprintf(newfilepath, "%s/%s", currentpath, argv[2]);

	if(rename(filepath, newfilepath)<0){
		printf("Failed to move: %s to %s\n", filepath, newfilepath);
		return FALSE;
	}

	printf("move file from: %s to %s\n", filepath, newfilepath);
	return TRUE;
}

//delete a directory
int rmdir_callback(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	sprintf(filepath, "%s/%s", currentpath, argv[1]);

	if(rmdir(filepath)<0){
		printf("Failed to remove: %s\n", filepath);
		return FALSE;
	}

	printf("remove: %s\n", filepath);
	return TRUE;
}

//make directory
int mkdir_callback(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("Parameter eror!\n");
		return FALSE;
	}

	sprintf(filepath, "%s/%s", currentpath, argv[1]);

	if(mkdir(filepath)<0){
		printf("Failed to create: %s\n", filepath);
		return FALSE;
	}

	printf("Create: %s success.\n", filepath);
	return TRUE;
}

