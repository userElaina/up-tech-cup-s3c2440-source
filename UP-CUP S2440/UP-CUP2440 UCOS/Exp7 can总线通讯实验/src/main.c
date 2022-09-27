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

	ARMTargetInit();	//�������ʼ��
  
	init_MCP2510(BandRate_250kbps);//���ڸú��������óɻػ�ģʽ
                                  //�Ӷ�ֻ��һ̨�豸���ʵ��
	canSetup();

	Uart_SendByte(0,0xa);//����
	Uart_SendByte(0,0xd);//�س�

	for(;;)
	{

		if(Uart_Poll(0)){	//�����յ����ݣ����͵�CAN����
			Uart_Getch((char*)data,0,0);
			canWrite(0x7ff, data, 8, FALSE, FALSE);
			
		}

		if((n=canPoll())!=-1){//CAN�����յ����ݣ����͵�����
			canRead(n, &id, data, &length,  &rxRTR, &isExt);
			Uart_SendByte(0,data[0]);	//��ʾ�ɼ�������
            Uart_SendByte(0,'\n');
            Uart_Printf(0,"id is %x",id);
            Uart_SendByte(0,'\n');
			if(data[0]=='\r')	//���ͻ���
				Uart_SendByte(0,'\n');
					
		}
	}
	return 0;
}