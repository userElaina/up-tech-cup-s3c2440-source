#include "../ucos-ii/includes.h"               /* uC/OS interface */
#include "../ucos-ii/add\list.h"
#include "../inc/sys/lib.h"
#include <stdio.h>
#include <string.h>

#if USE_MINIGUI==0

OS_MEM *pListMem;
INT8U ListMemPart[OSListMemNum][32];

void initOSList()
{
	INT8U err;

	pListMem=OSMemCreate(ListMemPart,OSListMemNum, 32, &err);
	if(pListMem==NULL){
		printk("Failed to Create List");
		LCD_printf("Failed to Create List");
	}
}

void AddListNode(PList pList, void* pNode)
{
	INT8U err;
	PList pNewList;
	
	pNewList=(PList)OSMemGet(pListMem,&err);
	pNewList->pData=pNode;
	pNewList->pNextList=pList->pNextList;
	pNewList->pPreList=pList;
	pList->pNextList=pNewList;
}

void DeleteListNode(PList pList)
{
	pList->pPreList->pNextList=pList->pNextList;

	if(pList->pNextList)
		pList->pNextList->pPreList=pList->pPreList;

	OSMemPut(pListMem, (void *) pList);
}

PList GetLastList(PList pList)
{
	PList ptmpList=pList;
	while(ptmpList->pNextList != NULL)
		ptmpList=ptmpList->pNextList;
	return ptmpList;
}

#endif //#if USE_MINIGUI==0

