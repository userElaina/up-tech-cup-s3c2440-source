/* 程序开始定义包含的头文件及定义需要的全局变量*/

#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include "pmplatform.h"
#include "Pkfuncs.h"
#include "BSP.h"

#define COMMAND_INIT		0x1
#define COMMAND_ARRAYSHOW	0x2
#define COMMAND_LEDSHOW		0x3
#define COMMAND_LEDOFF		0x4
#define COMMAND_ARRAYOFF	0x5
typedef struct{
	

	short unsigned int led[8];

	
}Dot;


typedef struct{
	short unsigned int high;
	short unsigned int low;
}Led;

Dot *dot=NULL;
Dot *temp=NULL;
Led *ledshow=NULL;
unsigned int LedData[]={0xc0,0xF9,0xA4,0xB0,0x99 ,0x92,0x82,0xF8,0x80,0x90,0x88 ,0x83,0xC6,0xA1,0x86,0x8E};
unsigned int DotLetter[10][4]={  //点阵数码管 数字字模
	{0xfe,0x82,0x82,0xfe}, 
	{0x00,0x00,0x00,0xfe},
	{0xf2,0x92,0x92,0x9e},
	{0x92,0x92,0x92,0xfe},
	{0x1e,0x10,0x10,0xfe},
	{0x9e,0x92,0x92,0xf2},
	{0xfe,0x92,0x92,0xf2}, 
	{0x02,0x02,0x02,0xfe},
	{0xfe,0x92,0x92,0xfe},
	{0x9e,0x92,0x92,0xfe}  	
};
/* 驱动动态库入口函数*/
BOOL WINAPI DllEntry (HANDLE hInstDll,DWORD dwReason,LPVOID lpvReserved)

{
 switch(dwReason)
 {
   case DLL_PROCESS_ATTACH:
  RETAILMSG(1,(TEXT("Digital_Dot:DLL_PROCESS_ATTACH.\r\n")));
  //DisableThreadLibraryCalls((HMODULE)hInstDll);
  break;//挂载成功

   case DLL_PROCESS_DETACH:
  RETAILMSG(1,(TEXT("Digital_Dot:DLL_PROCESS_DETACH.\r\n")));
  break;//卸载成功

 }

 return  (TRUE);

}

/*流驱动接口函数的初始化函数，该函数的主要工作是进行外部中断引脚的初始化，申请的逻辑中断号并保存到全局变量*/

 DWORD DDT_Init(DWORD dwContext)
{
	
	

	int i=0;
	dot = (Dot *)VirtualAlloc(0, sizeof(Dot), MEM_RESERVE, PAGE_NOACCESS);

		if (dot == NULL) 
		{
				
			RETAILMSG(1,(TEXT("For Digital_Dot : VirtualAlloc failed!\r\n")));
			return 0;
		}
		else 
		{
			if (!VirtualCopy((PVOID)dot, (PVOID)(0x08000000>>8), sizeof(Dot), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE)) 
			{
				RETAILMSG(1,(TEXT("For Digital_Dot: VirtualCopy failed!\r\n")));
				return 0;
			}
		}
		RETAILMSG(1,(TEXT("Digital_Dot:size of ledarray is %d\r\n"),sizeof(short unsigned int)));
//		ledarr+=118;
		temp=dot;
		temp=(Dot *)((unsigned char *)dot+0x100);
		//ledarr->led[0]=0xaa;
		ledshow=(Led *)((unsigned char *)dot+0x110);
		for( i=0;i<8;i++)
			{
				temp->led[i]=0x0;
			}
		temp->led[0]=0x1c;
		temp->led[1]=0x22;
		temp->led[2]=0x22;
		temp->led[3]=0x14;
		temp->led[4]=0x3E;
		temp->led[5]=0x2a;
		temp->led[6]=0x2a;
		temp->led[7]=0x22;

		ledshow->high=0xC6;
		ledshow->low=0x86;
		
		 Sleep(1500);
		  
		
		RETAILMSG(1,(TEXT("For Digital_Dot: init is ok!\r\n")));
		
	return 1;

 
}

 

 


DWORD DDT_Open(DWORD hDeviceContext,DWORD AccessCode,DWORD ShareMode)
{
 RETAILMSG(1,(TEXT("Digital_Dot:DDT_Open.\r\n")));
	return TRUE;
}


DWORD DDT_Read(DWORD Handle,LPVOID pBuffer,DWORD dwNumBytes)
{
	RETAILMSG(1,(TEXT("Digital_Dot:DDT_Read.\r\n")));
 
	return TRUE;
}


BOOL DDT_Close(DWORD Handle)
{
	RETAILMSG(1,(TEXT("Digital_Dot:DDT_Close.\r\n")));
	return TRUE;
 
}//LED_Close

 

 

BOOL DDT_Deinit(DWORD dwContext)
{
 

 return TRUE;
}


DWORD DDT_Write(DWORD hOpenContext,LPCVOID pBuffer,DWORD Count)
{ 
 return TRUE;
}

DWORD DDT_Seek(DWORD hOpenContext,long Amount ,DWORD Type)
{ 
 return TRUE;
}


BOOL DDT_IOControl(DWORD hOpenContext,
     DWORD dwCode ,
     PBYTE pBufIn,
     DWORD dwLenIn,
     PBYTE pBufOut,
     DWORD dwLenOut,
     PDWORD pdwActualOut)
{ 

	//RETAILMSG(1,(TEXT("For Digital_Dot: DDT_IOControl!\r\n")));
	int i=0,j=0;
	switch(dwCode)
	{
		case COMMAND_INIT:
			RETAILMSG(1,(TEXT("For Digital_Dot: init CE!\r\n")));
			temp->led[0]=0x1c;
			temp->led[1]=0x22;
			temp->led[2]=0x22;
			temp->led[3]=0x14;
			temp->led[4]=0x3E;
			temp->led[5]=0x2a;
			temp->led[6]=0x2a;
			temp->led[7]=0x22;
			
			ledshow->high=0xC6;
			ledshow->low=0x86;
			break;
		case COMMAND_ARRAYSHOW:
			RETAILMSG(1,(TEXT("For Digital_Dot: COMMAND_ARRAYSHOW!\r\n")));
			
			for(i=0;i<4;i++)
			temp->led[i]=DotLetter[*pBufIn/10][i];  //计算高位
			for(i=4;i<8;i++)
			{
				temp->led[i]=DotLetter[*pBufIn%10][j];  //计算低位
				j++;
			}
	
			
			break;

		case COMMAND_LEDSHOW:
			RETAILMSG(1,(TEXT("For Digital_Dot: COMMAND_LEDSHOW!\r\n")));

			ledshow->high=LedData[*pBufIn/10];
			ledshow->low=LedData[*pBufIn%10];
			break;

		case COMMAND_LEDOFF:
			RETAILMSG(1,(TEXT("For Digital_Dot: COMMAND_LEDOFF!\r\n")));
			ledshow->high=0xff;
			ledshow->low=0xff;
			break;

		case COMMAND_ARRAYOFF:
			RETAILMSG(1,(TEXT("For Digital_Dot: COMMAND_ARRAYOFF!\r\n")));
			for( i=0;i<8;i++)
			{
				temp->led[i]=0x0;
			}


			break;

		default :
			break;

	}
 return TRUE;
}




