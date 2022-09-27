#include	"..\ucos-ii\includes.h"               /* uC/OS interface */
#include "..\ucos-ii\add\OSMessage.h"

#if USE_MINIGUI==0

OS_EVENT *OSMessage_Que;		//系统消息队列
void *QOSMsg[OSMessageQSize];		//系统消息队列缓冲区

OS_MEM *pMessageMem;
INT8U MessageMemPart[OSMessageQSize][20];

void initOSMessage()
{
	INT8U err;
	OSMessage_Que=OSQCreate(&QOSMsg[0], OSMessageQSize);//创建系统的消息队列

	pMessageMem=OSMemCreate(MessageMemPart,OSMessageQSize, 20, &err);
	if(pMessageMem==NULL){
		LCD_printf("Failed to Create Message quote\n");
	}
}

POSMSG OSCreateMessage(POS_Ctrl pOSCtrl, U32 Message, U32 wparam, U32 lparam)
{
	POSMSG pmsg;
	INT8U err;
	pmsg=(POSMSG)OSMemGet(pMessageMem,&err);
	if(err!=OS_NO_ERR)
		return NULL;
	pmsg->pOSCtrl=pOSCtrl;
	pmsg->Message=Message;
	pmsg->WParam=wparam;
	pmsg->LParam=lparam;
	return pmsg;
}

U8 SendMessage(POSMSG pMsg)
{
	if(!pMsg)
		return FALSE;
	if(OSQPost(OSMessage_Que,pMsg)==OS_Q_FULL){
		DeleteMessage(pMsg);
		return FALSE;
	}
	return TRUE;
}

POSMSG WaitMessage(INT16U timeout)
{
	INT8U err;
	POSMSG pMsg=(POSMSG)OSQPend(OSMessage_Que,timeout,&err);
	return pMsg;
}

void DeleteMessage(POSMSG pMsg)
{
	if(pMsg)
		OSMemPut(pMessageMem,(void*)pMsg);
}

#endif //#if USE_MINIGUI==0

