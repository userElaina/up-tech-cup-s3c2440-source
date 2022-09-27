#include "../inc/drivers.h"
#include "../inc/lib.h"
#include <string.h>
#include <stdio.h>
#include "../inc/MCP2510.h"
#include "../inc/myuart.h"


#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 

int main(void)
{
	int n;
	unsigned int id;
	int length;
	BOOL rxRTR, isExt;
	U8 data[8]={0,};

	ARMTargetInit();	//开发版初始化
  
	init_MCP2510(BandRate_250kbps);//可在该函数内设置成回环模式
                                  //从而只用一台设备完成实验
	canSetup();

	Uart_SendByte(0,0xa);//换行
	Uart_SendByte(0,0xd);//回车

	for(;;)
	{

		if(Uart_Poll(0)){	//串口收到数据，则发送到CAN总线
			Uart_Getch((char*)data,0,0);
			canWrite(0x7ff, data, 8, FALSE, FALSE);
			
		}

		if((n=canPoll())!=-1){//CAN总线收到数据，则发送到串口
			canRead(n, &id, data, &length,  &rxRTR, &isExt);
			Uart_SendByte(0,data[0]);	//显示采集的数据
            Uart_SendByte(0,'\n');
            Uart_Printf(0,"id is %x",id);
            Uart_SendByte(0,'\n');
			if(data[0]=='\r')	//发送换行
				Uart_SendByte(0,'\n');
					
		}
	}
	return 0;
}