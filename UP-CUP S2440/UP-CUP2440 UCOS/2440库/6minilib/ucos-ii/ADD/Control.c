#include "../ucos-ii/includes.h"               /* uC/OS interface */
#include "../ucos-ii/add/OSaddition.h"
#include "../inc/drv/tchScr.h"
#include "../inc/macro.h"
#include "../inc/sys/lib.h"
#include "Ustring.h"
#include <stdio.h>
#include <string.h>

#if USE_MINIGUI==0

#define ROLLBUTTON_WIDTH		16

OS_MEM *pCtrlMem;
INT8U CtrlMemPart[OSCtrlMemSize][OSCtrlblkSize];

List OSCtrl;	//系统控件的链表，表头保留
U32 OSCtrlFocusID=NULL;		//输入焦点的控件ID

PDC pOSDC;	//系统的桌面DC
OS_EVENT *OSDC_Ctrl_mem;                   //获得系统DC控制设备上下文
//    OSDC_Ctrl_mem=OSSemCreate(1);       
//    OSSemPend(OSDC_Ctrl_mem,0,&err);
//    OSSemPost(OSDC_Ctrl_mem);

extern int OSFontSize[4];
const char arrowpicture[4][7]={0xff,0xff,0xff, 0, 0xff,0xff,0xff,
							      0xff,0xff, 0,0,0,0xff,0xff,
							            0xff,0,0,0,0,0,0xff,
							                 0,0,0,0,0,0,0};
void OSListCtrlOnMessage(PListCtrl pListCtrl, POSMSG pMsg);
void OSSendAllListMessage(PList pOsCtrl,POSMSG pMsg);
void OSTextCtrlOnMessage(PTextCtrl pTextCtrl, POSMSG pMsg);
void OSButtonCtrlOnMessage(PButtonCtrl pButtonCtrl,POSMSG pMsg);
void OSWndOnMessage(PWnd pWnd, POSMSG pMsg);

void initOSCtrl()
{
	INT8U err;

	pCtrlMem=OSMemCreate(CtrlMemPart,OSCtrlMemSize, OSCtrlblkSize, &err);
	if(pCtrlMem==NULL){
		printk("Failed to Create Message quote");
		LCD_printf("Failed to Create Message quote");
	}
	
	OSCtrl.pNextList=NULL;
	OSCtrl.pPreList=NULL;
	OSCtrl.pData=NULL;

	pOSDC=CreateDC();

	OSDC_Ctrl_mem=OSSemCreate(1);
}

void ReDrawListOsCtrl(PList pOsCtrl)
{
	OS_Ctrl* pCtrl;
	while(pOsCtrl->pNextList!= NULL){
		pOsCtrl=pOsCtrl->pNextList;
		pCtrl=(OS_Ctrl*)pOsCtrl->pData;

		if(!(pCtrl->bVisible))
			continue;

		switch(pCtrl->CtrlType){
		case CTRLTYPE_LISTCTRL:
			DrawListCtrl((PListCtrl)pCtrl);
			break;
		case CTRLTYPE_TEXTCTRL:
			DrawTextCtrl((PTextCtrl)pCtrl);
			break;
		case CTRLTYPE_PICTURECTRL:
			DrawPictureCtrl((PPictureCtrl)pCtrl);
			break;
		case CTRLTYPE_WINDOW:
			DrawWindow((PWnd)pCtrl);
			break;
		case CTRLTYPE_BUTTONCTRL:
			DrawButton((PButtonCtrl)pCtrl);
			break;
		}
	}
}

void ReDrawOSCtrl()	//绘制所有的操作系统的控件
{
	ReDrawListOsCtrl(&OSCtrl);
}

OS_Ctrl* GetCtrlfromID(PWnd pWnd, U32 ctrlID)
{
	OS_Ctrl* pCtrl;
	PList pOsCtrl;
	if(pWnd)
		pOsCtrl=&pWnd->ChildWndList;
	else
		pOsCtrl=&OSCtrl;

	while(pOsCtrl->pNextList!= NULL){
		pOsCtrl=pOsCtrl->pNextList;
		pCtrl=(OS_Ctrl*)pOsCtrl->pData;
		if(pCtrl->CtrlID==ctrlID)
			return pCtrl;
	}
	return NULL;
}

U32 SetWndCtrlFocus(PWnd pWnd, U32 CtrlID)
{
	U32 preID;
	if(pWnd){
		preID=pWnd->FocusCtrlID;
		pWnd->FocusCtrlID=CtrlID;
		return preID;
	}
	preID=OSCtrlFocusID;
	OSCtrlFocusID=CtrlID;
	return preID;
}

U32 GetWndCtrlFocus(PWnd pWnd)
{
	if(pWnd)
		return pWnd->FocusCtrlID;

	return OSCtrlFocusID;
}

BOOLEAN IsCtrlFocus(POS_Ctrl pCtrl)
{
	return (GetWndCtrlFocus(pCtrl->parentWnd)==pCtrl->CtrlID);
}
	
PDC GetCtrlParentDC(POS_Ctrl pCtrl)
{
	INT8U err;
	if(pCtrl->parentWnd){
		if(pCtrl->parentWnd->bVisible==FALSE)
			return NULL;
		OSSemPend(pCtrl->parentWnd->WndDC_Ctrl_mem,0,&err);
		return pCtrl->parentWnd->pdc;
	}

	OSSemPend(OSDC_Ctrl_mem,0,&err);
	return pOSDC;
}

void ReleaseCtrlParentDC(POS_Ctrl pCtrl)
{
	if(pCtrl->parentWnd){
		OSSemPost(pCtrl->parentWnd->WndDC_Ctrl_mem);
		return;
	}
	OSSemPost(OSDC_Ctrl_mem);
}

void Client2Screen(PWnd pWnd, structRECT* prect)
{
	while(pWnd){
		RectOffSet(prect, pWnd->ClientRect.left, pWnd->ClientRect.top);
		pWnd=pWnd->parentWnd;
	}
}


OS_Ctrl* CreateOSCtrl(U32 CtrlID, U32 CtrlType, structRECT* prect, U32 FontSize, U32 style, PWnd parentWnd)
{
	INT8U err;
	OS_Ctrl* pCtrl;
	PList pListEnd;
	
	pCtrl=(OS_Ctrl*)OSMemGet(pCtrlMem,&err);

	pCtrl->CtrlType=CtrlType;
	pCtrl->CtrlID=CtrlID;
	CopyRect(&pCtrl->ListCtrlRect,prect);
	pCtrl->FontSize=FontSize;
	pCtrl->style=style;
	pCtrl->bVisible=TRUE;
	pCtrl->parentWnd=parentWnd;
	pCtrl->CtrlMsgCallBk=NULL;


	if(parentWnd){
		pListEnd=GetLastList(&parentWnd->ChildWndList);
	}
	else	{
		pListEnd=GetLastList(&OSCtrl);	//添加列表控件到系统控件的链表中
	}
	AddListNode(pListEnd, (void*)pCtrl);

	return pCtrl;
}

void SetCtrlMessageCallBk(POS_Ctrl pOSCtrl, U8 (*CtrlMsgCallBk)(void*))
{
	pOSCtrl->CtrlMsgCallBk=CtrlMsgCallBk;
}


void ShowCtrl(OS_Ctrl *pCtrl, U8 bVisible)
{
	pCtrl->bVisible=bVisible;
}

PList GetCtrlList(PWnd pwnd, U32 CtrlID)
{
	OS_Ctrl* pCtrl;
	PList pOsCtrl;

	if(!pwnd)
		pOsCtrl=&OSCtrl;
	else
		pOsCtrl=&pwnd->ChildWndList;

	while(pOsCtrl->pNextList!= NULL){
		pOsCtrl=pOsCtrl->pNextList;
		pCtrl=(OS_Ctrl*)pOsCtrl->pData;
		if(pCtrl->CtrlID==CtrlID)
			return pOsCtrl;
	}
	return NULL;
}


PListCtrl CreateListCtrl(U32 CtrlID, structRECT* prect, int MaxNum, U32 FontSize, U32 style, PWnd parentWnd)
{
	PListCtrl pListCtrl;
	int i;
	INT8U err;
	
	pListCtrl=(PListCtrl)CreateOSCtrl(CtrlID,CTRLTYPE_LISTCTRL,prect, FontSize,style,parentWnd);

	pListCtrl->ListMaxNum=MaxNum;
	pListCtrl->CurrentSel=0;
	pListCtrl->pListText=OSMemGet(pCtrlMem,&err);
	for(i=0;i<MaxNum;i++){
		pListCtrl->pListText[i]=(U16*)OSMemGet(pCtrlMem,&err);
	}
	pListCtrl->CurrentHead=0;
	pListCtrl->ListNum=0;
	pListCtrl->ListShowNum=(prect->bottom-prect->top)/OSFontSize[FontSize];

	return pListCtrl;
}

void DestoryListCtrl(PListCtrl plistCtrl)
{
	PList pList=GetCtrlList(plistCtrl->parentWnd, plistCtrl->CtrlID);
	DeleteListNode(pList);
	OSMemPut(pCtrlMem, (void*)plistCtrl);
}

U8 AddStringListCtrl(PListCtrl pListCtrl, U16 string[])
{
	int i;
	if(pListCtrl->ListNum+1>pListCtrl->ListMaxNum)
		return FALSE;

	for(i=0;i<OSCtrlblkSize/2 && string[i]!=0;i++)
		pListCtrl->pListText[pListCtrl->ListNum][i]=string[i];

	pListCtrl->pListText[pListCtrl->ListNum][i]=0;
	pListCtrl->ListNum++;
	return TRUE;
}

void ListCtrlReMoveAll(PListCtrl pListCtrl)
{
	pListCtrl->ListNum=0;
	pListCtrl->CurrentHead=0;
	pListCtrl->CurrentSel=0;
}

void ReLoadListCtrl(PListCtrl pListCtrl,U16* string[],int nstr)
{
	int i;

	ListCtrlReMoveAll(pListCtrl);
	for(i=0;i<nstr;i++)
		AddStringListCtrl(pListCtrl, string[i]);
}

void DrawRollArrow(PDC pdc,int x, int y, BOOLEAN bUp, BOOLEAN bEnable)
{
	int i,j;
	COLORREF color= RGB(0,0,0);
	if(!bEnable)
		color=RGB(192,192,192);

	if(bUp){
		for(j=0;j<4;j++){
			for(i=0;i<7;i++){
				if(arrowpicture[j][i])
					SetPixel(pdc, x+i, y+j, RGB(255,255,255));
				else
					SetPixel(pdc, x+i, y+j, color);
			}
		}
	}
	else{
		for(j=0;j<4;j++){
			for(i=0;i<7;i++){
				if(arrowpicture[j][i])
					SetPixel(pdc, x+i, y+6-j, RGB(255,255,255));
				else
					SetPixel(pdc, x+i, y+6-j, color);
			}
		}
	}
}

void OSSendAllListMessage(PList pOsCtrl,POSMSG pMsg)
{
	OS_Ctrl* pCtrl;
	while(pOsCtrl->pNextList!= NULL){
		pOsCtrl=pOsCtrl->pNextList;
		pCtrl=(OS_Ctrl*)pOsCtrl->pData;

		if(!(pCtrl->bVisible))
			continue;

		if(pCtrl->CtrlMsgCallBk){
			if((*pCtrl->CtrlMsgCallBk)(pMsg))
				continue;
		}

		switch(pCtrl->CtrlType){
		case CTRLTYPE_LISTCTRL:
			OSListCtrlOnMessage((PListCtrl) pCtrl, pMsg);
			break;
		case CTRLTYPE_TEXTCTRL:
			OSTextCtrlOnMessage((PTextCtrl) pCtrl, pMsg);
			break;
		case CTRLTYPE_PICTURECTRL:
			break;
		case CTRLTYPE_WINDOW:
			OSWndOnMessage((PWnd) pCtrl, pMsg);
			break;
		case CTRLTYPE_BUTTONCTRL:
			OSButtonCtrlOnMessage((PButtonCtrl) pCtrl, pMsg);
			break;
		}
	}
}

void OSOnSysMessage(void* pMsg)
{
	OS_Ctrl *pCtrl;
	if(((POSMSG)pMsg)->Message==OSM_TOUCH_SCREEN){
		pCtrl=GetCtrlfromID(NULL, GetWndCtrlFocus(NULL));
		if(pCtrl){
			if(pCtrl->CtrlType==CTRLTYPE_WINDOW){
				if((((PWnd)pCtrl)->style&WND_STYLE_MODE)==WND_STYLE_MODE){
					//焦点是有模式窗口，消息直接传递过去
					OSSendAllListMessage(&((PWnd)pCtrl)->ChildWndList, pMsg);
					return;
				}
			}
		}
	}
	OSSendAllListMessage(&OSCtrl, (POSMSG)pMsg);
}

//绘制列表框的滚动条
void ListCtrlDrawRollBar(PListCtrl pListCtrl, PDC pdc)
{
	structRECT rect;
	int length1,length2;


	rect.left=pListCtrl->ListCtrlRollRect.left;
	rect.top=pListCtrl->ListCtrlRollRect.top;
	rect.right=pListCtrl->ListCtrlRollRect.right;
	rect.bottom=pListCtrl->ListCtrlRollRect.top+ROLLBUTTON_WIDTH;

	length1=GetRectHeight(&pListCtrl->ListCtrlRollRect)-ROLLBUTTON_WIDTH*2;

	//绘制滚动条向上的箭头
	FillRect2(pdc, &rect, NULL, RGB(255,255,255));
	Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(0,0,0));
	InflateRect(&rect, -1, -1);
	Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(128,128,128));
	DrawRollArrow(pdc,rect.left+4,rect.top+4,TRUE,TRUE);
	
	//绘制滚动条中间
	rect.left-=1;
	rect.right+=1;
	rect.top=rect.bottom+1;
	rect.bottom=pListCtrl->ListCtrlRollRect.bottom-ROLLBUTTON_WIDTH-1;
	FillRect2(pdc, &rect, NULL, RGB(128,128,128));

	//绘制滚动条滑块
	rect.top+=pListCtrl->CurrentHead*length1/pListCtrl->ListNum;
	length2=pListCtrl->ListShowNum*length1/pListCtrl->ListNum;
	length2=min(length1,length2);
	length2=max(6,length2);
	rect.bottom=rect.top+length2;
	CopyRect(&pListCtrl->RollBlockRect, &rect);

	FillRect2(pdc, &rect, NULL, RGB(255,255,255));
	Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(0,0,0));
	InflateRect(&rect, -1, -1);
	Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(128,128,128));

	//绘制滚动条向下的箭头
	rect.left-=1;
	rect.right+=1;
	rect.top=pListCtrl->ListCtrlRollRect.bottom-ROLLBUTTON_WIDTH;
	rect.bottom=pListCtrl->ListCtrlRollRect.bottom;
	Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(0,0,0));
	InflateRect(&rect, -1, -1);
	Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(128,128,128));
	DrawRollArrow(pdc,rect.left+4,rect.top+4,FALSE,TRUE);

}

void ListCtrlDrawClient(PListCtrl pListCtrl, PDC pdc)
{
	structRECT textRect;
	int i;

	for(i=0;i<pListCtrl->ListShowNum;i++){
		SetRect(&textRect, pListCtrl->ClientRect.left, 
			pListCtrl->ClientRect.top+i*OSFontSize[pListCtrl->FontSize],
			pListCtrl->ClientRect.right,pListCtrl->ClientRect.top+(i+1)*OSFontSize[pListCtrl->FontSize]);
		if(textRect.bottom>pListCtrl->ListCtrlRect.bottom)
			break;
		if(i+pListCtrl->CurrentHead>=pListCtrl->ListNum)
			break;
		if(i+pListCtrl->CurrentHead==pListCtrl->CurrentSel)	//文本被选中
			TextOutRect(pdc, &textRect, pListCtrl->pListText[i+pListCtrl->CurrentHead],
				TRUE, pListCtrl->FontSize|FONT_BLACKBK,TEXTOUT_MID_Y);
		else
			TextOutRect(pdc,&textRect, pListCtrl->pListText[i+pListCtrl->CurrentHead],
				TRUE, pListCtrl->FontSize,TEXTOUT_MID_Y);
	}

}

void DrawListCtrl(PListCtrl pListCtrl)
{
	structRECT rect;
	PDC pdc;

	pdc=GetCtrlParentDC((POS_Ctrl)pListCtrl);
	if(!pdc)
		return;

	//清空列表框的显示区域
	FillRect2(pdc,&pListCtrl->ListCtrlRect,GRAPH_MODE_NORMAL,COLOR_WHITE);

	CopyRect(&rect, &pListCtrl->ListCtrlRect);

	switch(pListCtrl->style){
	case CTRL_STYLE_DBFRAME:
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(255,255,255), RGB(255,255,255));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(0,0,0), RGB(192,192,192));
		break;
	case CTRL_STYLE_FRAME:
		DrawRectFrame2(pdc,&rect);
		break;
	case CTRL_STYLE_3DUPFRAME:
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(128,128,128));
		break;
	case CTRL_STYLE_3DDOWNFRAME:
		Draw3DRect2(pdc, &rect, RGB(0,0,0),RGB(192,192,192));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
		break;
	case CTRL_STYLE_NOFRAME:
		break;
	}
	InflateRect(&rect, -1,-1);

	pListCtrl->ListCtrlRollRect.left=rect.right-ROLLBUTTON_WIDTH;
	pListCtrl->ListCtrlRollRect.top=rect.top;
	pListCtrl->ListCtrlRollRect.right=rect.right;
	pListCtrl->ListCtrlRollRect.bottom=rect.bottom;

	pListCtrl->ClientRect.left=rect.left;
	pListCtrl->ClientRect.top=rect.top;
	pListCtrl->ClientRect.right=rect.right-ROLLBUTTON_WIDTH;
	pListCtrl->ClientRect.bottom=rect.bottom;

	ListCtrlDrawClient(pListCtrl, pdc);
	ListCtrlDrawRollBar(pListCtrl, pdc);

	ReleaseCtrlParentDC((POS_Ctrl)pListCtrl);
}

void ListCtrlOnTchScr(PListCtrl pListCtrl, int x, int y, U32 tchaction)
{
	int sel;
	POSMSG pMsg;
	if(IsInRect(&pListCtrl->ClientRect,x,y)){
		//在客户区域内单击，改变列表框的选择
		PDC pdc;
		sel=(y-pListCtrl->ClientRect.top)/OSFontSize[pListCtrl->FontSize];
		if(pListCtrl->CurrentHead+sel+1>pListCtrl->ListNum)
			return;

		pdc=GetCtrlParentDC((POS_Ctrl)pListCtrl);
		pListCtrl->CurrentSel=pListCtrl->CurrentHead+sel;
		ListCtrlDrawClient(pListCtrl,pdc);
		ReleaseCtrlParentDC((POS_Ctrl)pListCtrl);

		switch(tchaction){
		case TCHSCR_ACTION_CLICK:
			pMsg=OSCreateMessage((POS_Ctrl)pListCtrl->parentWnd, OSM_LISTCTRL_SELCHANGE, pListCtrl->CtrlID, pListCtrl->CurrentSel);
			SendMessage(pMsg);
			break;
		case TCHSCR_ACTION_DBCLICK:
			pMsg=OSCreateMessage((POS_Ctrl)pListCtrl->parentWnd, OSM_LISTCTRL_SELDBCLICK, pListCtrl->CtrlID, pListCtrl->CurrentSel);
			SendMessage(pMsg);
			break;
		}
		return;
	}
	if(IsInRect(&pListCtrl->ListCtrlRollRect,x,y)){
		//在滚动条区域内
		int rolltop,rollbottom;
		structRECT rect;
		PDC pdc;
		rolltop=pListCtrl->ListCtrlRollRect.top;
		rollbottom=pListCtrl->ListCtrlRollRect.bottom;
		if(y<rolltop+ROLLBUTTON_WIDTH && tchaction!=TCHSCR_ACTION_MOVE){
			//向上移动一格
			rect.left=pListCtrl->ListCtrlRollRect.left;
			rect.top=pListCtrl->ListCtrlRollRect.top;
			rect.right=pListCtrl->ListCtrlRollRect.right;
			rect.bottom=pListCtrl->ListCtrlRollRect.top+ROLLBUTTON_WIDTH;


			switch(tchaction){
			case TCHSCR_ACTION_UP:
				pListCtrl->CurrentHead=max(0,pListCtrl->CurrentHead-1);
				DrawListCtrl(pListCtrl);
				break;
			case TCHSCR_ACTION_DOWN:
				pdc=GetCtrlParentDC((POS_Ctrl)pListCtrl);

				Draw3DRect2(pdc,&rect, RGB(0,0,0), RGB(192,192,192));
				InflateRect(&rect, -1, -1);
				Draw3DRect2(pdc,&rect, RGB(128,128,128),RGB(192,192,192));

				ReleaseCtrlParentDC((POS_Ctrl)pListCtrl);
				break;

			}

			return;
		}
		if(y<rollbottom && y>rollbottom-ROLLBUTTON_WIDTH && tchaction!=TCHSCR_ACTION_MOVE){
			//向下移动一格
			rect.left=pListCtrl->ListCtrlRollRect.left;
			rect.top=pListCtrl->ListCtrlRollRect.bottom-ROLLBUTTON_WIDTH;
			rect.right=pListCtrl->ListCtrlRollRect.right;
			rect.bottom=pListCtrl->ListCtrlRollRect.bottom;


			switch(tchaction){
/*			case TCHSCR_ACTION_CLICK:
				pdc=GetCtrlParentDC((POS_Ctrl)pListCtrl);
				Draw3DRect2(pdc,&rect, RGB(0,0,0), RGB(192,192,192));
				InflateRect(&rect, -1, -1);
				Draw3DRect2(pdc,&rect, RGB(128,128,128),RGB(192,192,192));
				ReleaseCtrlParentDC((POS_Ctrl)pListCtrl);*/
			case TCHSCR_ACTION_UP:
				pListCtrl->CurrentHead=min(pListCtrl->CurrentHead+1,pListCtrl->ListNum-pListCtrl->ListShowNum);
				pListCtrl->CurrentHead=max(0,pListCtrl->CurrentHead);
				DrawListCtrl(pListCtrl);
				break;
			case TCHSCR_ACTION_DOWN:
				pdc=GetCtrlParentDC((POS_Ctrl)pListCtrl);
				Draw3DRect2(pdc,&rect, RGB(0,0,0), RGB(192,192,192));
				InflateRect(&rect, -1, -1);
				Draw3DRect2(pdc,&rect, RGB(128,128,128),RGB(192,192,192));
				ReleaseCtrlParentDC((POS_Ctrl)pListCtrl);
				break;

			}
			return;
		}
		if(IsInRect(&pListCtrl->RollBlockRect, x, y)|| tchaction==TCHSCR_ACTION_MOVE ){	//拖动滑块
			int head,preCurhead=pListCtrl->CurrentHead,
				length1=GetRectHeight(&pListCtrl->ListCtrlRollRect)
					-ROLLBUTTON_WIDTH*2-GetRectHeight(&pListCtrl->RollBlockRect),
				freenum=pListCtrl->ListNum-pListCtrl->ListShowNum;
			static int oldy;
			if(freenum>0 && tchaction==TCHSCR_ACTION_MOVE){	//拖动滑块
				head=(y-oldy)*freenum/length1;
				preCurhead=min(preCurhead+head,freenum);
				preCurhead=max(0,preCurhead);
				if(preCurhead!=pListCtrl->CurrentHead){
					oldy=y;
					pListCtrl->CurrentHead=preCurhead;
					DrawListCtrl(pListCtrl);
				}
			}
			else if(tchaction==TCHSCR_ACTION_DOWN){
				oldy=y;
			}
			return;
		}
		if(y<pListCtrl->RollBlockRect.top && tchaction==TCHSCR_ACTION_CLICK){//向上翻页
			int preCurhead=pListCtrl->CurrentHead;
			preCurhead=max(preCurhead-pListCtrl->ListShowNum,0);
			if(preCurhead!=pListCtrl->CurrentHead){
				pListCtrl->CurrentHead=preCurhead;
				DrawListCtrl(pListCtrl);
			}
			return;
		}
		if(y>pListCtrl->RollBlockRect.bottom && tchaction==TCHSCR_ACTION_CLICK){//向下翻页
			int preCurhead=pListCtrl->CurrentHead;
			preCurhead=min(preCurhead+pListCtrl->ListShowNum,pListCtrl->ListNum-pListCtrl->ListShowNum);
			preCurhead=max(0,preCurhead);
			if(preCurhead!=pListCtrl->CurrentHead){
				pListCtrl->CurrentHead=preCurhead;
				DrawListCtrl(pListCtrl);
			}
		}
	}
}

void ListCtrlSelMove(PListCtrl pListCtrl, int moveNum, U8 Redraw)	//列表框高亮度条移，正数下移，负数上移
{
	POSMSG pMsg;
	int oldsel=pListCtrl->CurrentSel;

	if(moveNum<0){	//向上滚动
		if(pListCtrl->CurrentSel+moveNum>=pListCtrl->CurrentHead)
			pListCtrl->CurrentSel+=moveNum;
		else{
			pListCtrl->CurrentSel=max(pListCtrl->CurrentSel+moveNum,0);
			pListCtrl->CurrentHead=pListCtrl->CurrentSel;
		}
	}
	else{ //向下滚动
		if(pListCtrl->CurrentSel+moveNum<pListCtrl->CurrentHead+pListCtrl->ListShowNum 
			&& pListCtrl->CurrentSel+moveNum<(pListCtrl->ListNum-1))
			pListCtrl->CurrentSel+=moveNum;
		else{
			pListCtrl->CurrentSel=min(pListCtrl->CurrentSel+moveNum,pListCtrl->ListNum-1);
			pListCtrl->CurrentHead=max(pListCtrl->CurrentSel-pListCtrl->ListShowNum+1,0);
		}
	}
	if(oldsel==pListCtrl->CurrentSel)
		return;

	if(Redraw)
		DrawListCtrl(pListCtrl);
	//发送列表框选择改变消息
	pMsg=OSCreateMessage((POS_Ctrl)pListCtrl->parentWnd,OSM_LISTCTRL_SELCHANGE, pListCtrl->CtrlID, pListCtrl->CurrentSel);
	SendMessage(pMsg);
}

void ListCtrlOnKey(PListCtrl pListCtrl, int nkey, int fnkey)
{
	switch(nkey){
	case '8'|KEY_DOWN:	//move up
		ListCtrlSelMove(pListCtrl,-1,TRUE);
		return;
	case '2'|KEY_DOWN:	//move down
		ListCtrlSelMove(pListCtrl,1,TRUE);
		return;
	}

}


void OSListCtrlOnMessage(PListCtrl pListCtrl, POSMSG pMsg)
{
	switch(pMsg->Message){
	case OSM_TOUCH_SCREEN:
		ListCtrlOnTchScr(pListCtrl, (S16)(pMsg->WParam&0xffff), (S16)(pMsg->WParam>>16), pMsg->LParam);
		break;
	case OSM_KEY:
		if(IsCtrlFocus((POS_Ctrl)pListCtrl))
			ListCtrlOnKey(pListCtrl, pMsg->WParam,pMsg->LParam);
		break;
	}
}

char TextDefKeyTable[]={0,0,0,'\b','7','8','9',0,'4','5','6','1','2','3',0,'0','.'};

PTextCtrl CreateTextCtrl(U32 CtrlID, structRECT* prect,  U32 FontSize, U32 style,char* KeyTable, PWnd parentWnd)
{
	PTextCtrl pTextCtrl;

	pTextCtrl=(PTextCtrl)CreateOSCtrl(CtrlID,CTRLTYPE_TEXTCTRL,prect, FontSize,style, parentWnd);
	pTextCtrl->bIsEdit=FALSE;
	pTextCtrl->text[0]=NULL;

	if(KeyTable)
		pTextCtrl->KeyTable=KeyTable;
	else
		pTextCtrl->KeyTable=TextDefKeyTable;

	return pTextCtrl;
}

void DestoryTextCtrl(PTextCtrl pTextCtrl)
{
	PList pList=GetCtrlList(pTextCtrl->parentWnd, pTextCtrl->CtrlID);
	DeleteListNode(pList);
	OSMemPut(pCtrlMem, (void*)pTextCtrl);
}

void SetTextCtrlText(PTextCtrl pTextCtrl, U16 *pch, U8 IsRedraw)
{
	UstrCpy(pTextCtrl->text, pch);

	if(IsRedraw /*&& pTextCtrl->CtrlID==GetWndCtrlFocus(pTextCtrl->parentWnd)*/)
		DrawTextCtrl(pTextCtrl);
}

U16* GetTextCtrlText(PTextCtrl pTextCtrl)
{
	return pTextCtrl->text;
}

void DrawTextCtrl(PTextCtrl pTextCtrl)
{
	structRECT rect;
	PDC pdc;
	pdc=GetCtrlParentDC((POS_Ctrl)pTextCtrl);
	if(!pdc)
		return;

	//清空列表框的显示区域
	FillRect2(pdc, &pTextCtrl->TextCtrlRect,GRAPH_MODE_NORMAL, COLOR_WHITE);

	CopyRect(&rect, &pTextCtrl->TextCtrlRect);
	if(GetWndCtrlFocus(pTextCtrl->parentWnd)==pTextCtrl->CtrlID){	//当前的控件为焦点
		switch(pTextCtrl->style){
		case CTRL_STYLE_DBFRAME:
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(255,255,255), RGB(255,255,255));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(0,0,0), RGB(192,192,192));
			break;
		case CTRL_STYLE_FRAME:
			Draw3DRect2(pdc, &rect, RGB(0,0,0), RGB(0,0,0));
			break;
		case CTRL_STYLE_3DUPFRAME:
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(128,128,128));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
			break;
		case CTRL_STYLE_3DDOWNFRAME:
			Draw3DRect2(pdc, &rect, RGB(0,0,0),RGB(192,192,192));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
			break;
		case CTRL_STYLE_NOFRAME:
			break;
		}
	}
	else{
		switch(pTextCtrl->style){
		case CTRL_STYLE_DBFRAME:
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(0,0,0));
			break;
		case CTRL_STYLE_FRAME:
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(128,128,128));
			break;
		case CTRL_STYLE_3DUPFRAME:
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(128,128,128));
			break;
		case CTRL_STYLE_3DDOWNFRAME:
			Draw3DRect2(pdc, &rect, RGB(0,0,0),RGB(192,192,192));
			InflateRect(&rect, -1,-1);
			Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
			break;
		case CTRL_STYLE_NOFRAME:
			break;
		}
	}
	InflateRect(&rect, -1,-1);

	if(pTextCtrl->bIsEdit)	//文本框处于编辑状态
		TextOutRect(pdc, &rect, pTextCtrl->text, TRUE, 
			pTextCtrl->FontSize|FONT_BLACKBK,TEXTOUT_MID_Y);
	else
		TextOutRect(pdc, &rect, pTextCtrl->text, TRUE,
			pTextCtrl->FontSize,TEXTOUT_MID_Y);

	ReleaseCtrlParentDC((POS_Ctrl)pTextCtrl);
}

void AppendChar2TextCtrl(PTextCtrl pTextCtrl, U16 ch, U8 IsReDraw)
{
	int i;
	for(i=0;pTextCtrl->text[i];i++){
		if(i>39)
			return;
	}

	pTextCtrl->text[i++]=ch;
	pTextCtrl->text[i]=0;

	if(IsReDraw)
		DrawTextCtrl(pTextCtrl);
}

void TextCtrlDeleteChar(PTextCtrl pTextCtrl,U8 IsReDraw)
{
	int i;
	for(i=0;pTextCtrl->text[i];i++){
		if(i>39)
			return;
	}
	if(i==0)
		return;
	
	pTextCtrl->text[--i]=0;

	if(IsReDraw)
		DrawTextCtrl(pTextCtrl);
}

void SetTextCtrlEdit(PTextCtrl pTextCtrl, U8 bIsEdit)
{
	pTextCtrl->bIsEdit=bIsEdit;
}

void TextCtrlOnTchScr(PTextCtrl pListCtrl, int x, int y, U32 tchaction)
{
}

void	TextCtrlOnKey(PTextCtrl pTextCtrl, int nkey, int fnkey)
{
	static U16 tempbuffer[100]={0,};

	if((nkey&KEY_DOWN)==0)
		return;

	switch(nkey&0xff){
	case '\r'://OK
		if(!pTextCtrl->bIsEdit){
			UstrCpy(tempbuffer, pTextCtrl->text);
			pTextCtrl->text[0]=0;
			SetTextCtrlEdit(pTextCtrl, TRUE);
			DrawTextCtrl(pTextCtrl);
		}
		else{
			SetTextCtrlEdit(pTextCtrl, FALSE);
			DrawTextCtrl(pTextCtrl);
		}
		break;
	case '.'://cancel
		if(pTextCtrl->bIsEdit){
			UstrCpy(pTextCtrl->text,tempbuffer);
			SetTextCtrlEdit(pTextCtrl, FALSE);
			DrawTextCtrl(pTextCtrl);
		}
		break;
	default:{
//			U16 charactor=pTextCtrl->KeyTable[nkey];
			U16 charactor=(nkey&0xff);
			if(charactor==0 || (!pTextCtrl->bIsEdit))
				return;
			if(charactor=='-') //陈文华改动过
			//if(charactor=='\b')
				TextCtrlDeleteChar(pTextCtrl, TRUE);
			else
				AppendChar2TextCtrl(pTextCtrl,charactor,TRUE);
		}
	}
}


void OSTextCtrlOnMessage(PTextCtrl pTextCtrl, POSMSG pMsg)
{
	switch(pMsg->Message){
	case OSM_TOUCH_SCREEN:
		TextCtrlOnTchScr(pTextCtrl, (S16)(pMsg->WParam&0xffff), (S16)(pMsg->WParam>>16), pMsg->LParam);
		break;
	case OSM_KEY:
		if(IsCtrlFocus((POS_Ctrl)pTextCtrl))
			TextCtrlOnKey(pTextCtrl, pMsg->WParam,pMsg->LParam);
		break;
	}
}

PPictureCtrl CreatePictureCtrl(U32 CtrlID, structRECT* prect,  char filename[], U32 style, PWnd parentWnd)
{
	PPictureCtrl pPicutureCtrl;

	pPicutureCtrl=(PPictureCtrl)CreateOSCtrl(CtrlID,CTRLTYPE_PICTURECTRL,prect, NULL,style,parentWnd);
	strcpy(pPicutureCtrl->picfilename,filename);

	return pPicutureCtrl;
}

void DestoryPictureCtrl(PPictureCtrl pPictureCtrl)
{
	PList pList=GetCtrlList(pPictureCtrl->parentWnd, pPictureCtrl->CtrlID);
	DeleteListNode(pList);
	OSMemPut(pCtrlMem, (void*)pPictureCtrl);
}

void DrawPictureCtrl(PPictureCtrl pPictureCtrl)
{
	structRECT rect;
	int tmp,width,height;
	PDC pdc;
	pdc=GetCtrlParentDC((POS_Ctrl)pPictureCtrl);
	if(!pdc)
		return;

	//清空图片框的显示区域
	FillRect2(pdc, &pPictureCtrl->PictureCtrlRect,GRAPH_MODE_NORMAL, COLOR_WHITE);

	CopyRect(&rect, &pPictureCtrl->PictureCtrlRect);

	switch(pPictureCtrl->style){
	case CTRL_STYLE_DBFRAME:
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(255,255,255), RGB(255,255,255));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(0,0,0), RGB(192,192,192));
		break;
	case CTRL_STYLE_FRAME:
		DrawRectFrame2(pdc,&rect);
		break;
	case CTRL_STYLE_3DUPFRAME:
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(128,128,128));
		break;
	case CTRL_STYLE_3DDOWNFRAME:
		Draw3DRect2(pdc, &rect, RGB(0,0,0),RGB(192,192,192));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(192,192,192));
		break;
	case CTRL_STYLE_NOFRAME:
		break;
	}
	InflateRect(&rect, -1,-1);
	GetBmpSize(pPictureCtrl->picfilename, &width,&height);

	tmp=(rect.bottom-rect.top-height)/2;
	if(tmp>0){
		rect.top+=tmp;
	}

	tmp=(rect.right-rect.left-width)/2;
	if(tmp>0){
		rect.left+=tmp;
	}
	ShowBmp(pdc, pPictureCtrl->picfilename, rect.left, rect.top);

	ReleaseCtrlParentDC((POS_Ctrl)pPictureCtrl);
}

PWnd CreateWindow(U32 CtrlID, structRECT* prect,  U32 FontSize, U32 style, U16 Caption[], PWnd parentWnd)
{
	PWnd pwnd;

	pwnd=(PWnd)CreateOSCtrl(CtrlID,CTRLTYPE_WINDOW,prect, FontSize,style,parentWnd);
	UstrCpy(pwnd->Caption,Caption);

	pwnd->ChildWndList.pNextList=NULL;
	pwnd->ChildWndList.pPreList=NULL;
	pwnd->ChildWndList.pData=NULL;

	pwnd->pdc=CreateDC();
	pwnd->pdc->WndOrgx=prect->left;
	pwnd->pdc->WndOrgy=prect->top;

	pwnd->FocusCtrlID=NULL;
	pwnd->preParentFocusCtrlID=NULL;
	pwnd->bVisible=FALSE;

	pwnd->WndDC_Ctrl_mem=OSSemCreate(1);

	return pwnd;
}

void DrawWindow(PWnd pwnd)
{
	PDC pdc;
	structRECT rect,rect1;

	if(pwnd==NULL){
		ReDrawOSCtrl();
		return;
	}

	pdc=GetCtrlParentDC((POS_Ctrl)pwnd);
	if(!pdc)
		return;

	//清空图片框的显示区域
	FillRect2(pdc, &pwnd->WndRect,GRAPH_MODE_NORMAL, COLOR_WHITE);

	CopyRect(&rect, &pwnd->WndRect);

	switch(pwnd->style&0xffff){
	case CTRL_STYLE_DBFRAME:
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(0,0,0));
		InflateRect(&rect, -1,-1);
		break;
	case CTRL_STYLE_FRAME:
		DrawRectFrame2(pdc,&rect);
		break;
	case CTRL_STYLE_NOFRAME:
		break;
	}
	InflateRect(&rect, -1,-1);

	if(pwnd->style&WND_STYLE_TITLE){
		rect1.left=rect.left;
		rect1.top=rect.top;
		rect1.right=rect.right;
		rect1.bottom=rect1.top+GetFontHeight(pwnd->FontSize);

		rect.top+=GetFontHeight(pwnd->FontSize)+1;

		FillRect2(pdc, &rect1, NULL , COLOR_BLACK);
		TextOutRect(pdc, &rect1, pwnd->Caption, TRUE, FONT_BLACKBK,TEXTOUT_MID_Y);
	}
		
	CopyRect(&pwnd->ClientRect, &rect);
	pwnd->pdc->WndOrgx=rect.left;
	pwnd->pdc->WndOrgy=rect.top;
	CopyRect(&pwnd->pdc->DrawRect, &rect);

	ReleaseCtrlParentDC((POS_Ctrl)pwnd);

	ReDrawListOsCtrl(&pwnd->ChildWndList);

	
}

void ShowWindow(PWnd pwnd, BOOLEAN isShow)
{
	if(pwnd->bVisible==isShow)
		return;

	pwnd->bVisible=isShow;
	if(isShow){
		//设置焦点到当前窗口
		pwnd->preParentFocusCtrlID=SetWndCtrlFocus(pwnd->parentWnd, pwnd->CtrlID);
		DrawWindow(pwnd);
	}
	else{
		PDC pdc=CreateDC();
		FillRect2(pdc, &pwnd->WndRect,GRAPH_MODE_NORMAL, COLOR_WHITE);
		DestoryDC(pdc);

		//还原原来的父窗口焦点
		SetWndCtrlFocus(pwnd->parentWnd, pwnd->preParentFocusCtrlID);
		DrawWindow(pwnd->parentWnd);
	}
}

void WndOnTchScr(PWnd pWnd, int x,int y, U32 tchaction)
{
/*	if(IsInRect(&pWnd->ClientRect, x, y)){
		OSCtrlOnTouchScr(&pWnd->ChildWndList, 
			x-pWnd->ClientRect.left, y-pWnd->ClientRect.top, tchaction);
	}*/
}

void DestoryWindow(PWnd pWnd)
{
	OS_Ctrl* pCtrl;
	PList pOsCtrl=&pWnd->ChildWndList;

	ShowWindow(pWnd, FALSE);

	while(pOsCtrl->pNextList!= NULL){
		pOsCtrl=pOsCtrl->pNextList;
		pCtrl=(OS_Ctrl*)pOsCtrl->pData;

		if(!(pCtrl->bVisible))
			continue;


		switch(pCtrl->CtrlType){
		case CTRLTYPE_LISTCTRL:
			DestoryListCtrl((PListCtrl)pCtrl);
			break;
		case CTRLTYPE_TEXTCTRL:
			DestoryTextCtrl((PTextCtrl)pCtrl);
			break;
		case CTRLTYPE_PICTURECTRL:
			DestoryPictureCtrl((PPictureCtrl)pCtrl);
			break;
		case CTRLTYPE_WINDOW:
			DestoryWindow((PWnd)pCtrl);
			break;
		case CTRLTYPE_BUTTONCTRL:
			DestoryButton((PButtonCtrl)pCtrl);
			break;
		}
	}

	DestoryDC(pWnd->pdc);

	pOsCtrl=GetCtrlList(pWnd->parentWnd, pWnd->CtrlID);
	DeleteListNode(pOsCtrl);
	OSMemPut(pCtrlMem, (void*)pWnd);
}

void OSWndOnMessage(PWnd pWnd, POSMSG pMsg)
{
	OS_Ctrl *pCtrl;
	switch(pMsg->Message){
	case OSM_TOUCH_SCREEN:
		WndOnTchScr(pWnd, (S16)(pMsg->WParam&0xffff), (S16)(pMsg->WParam>>16), pMsg->LParam);

		pCtrl=GetCtrlfromID(pWnd, GetWndCtrlFocus(pWnd));
		if(pCtrl){
			if(pCtrl->CtrlType==CTRLTYPE_WINDOW){
				if((((PWnd)pCtrl)->style&WND_STYLE_MODE)==WND_STYLE_MODE){
					//焦点是有模式窗口，消息直接传递过去
					OSSendAllListMessage(&((PWnd)pCtrl)->ChildWndList, pMsg);
					return;
				}
			}
		}
		OSSendAllListMessage(&pWnd->ChildWndList, pMsg);
		break;
	case OSM_KEY:
		if(IsCtrlFocus((POS_Ctrl)pWnd))
			OSSendAllListMessage(&pWnd->ChildWndList, pMsg);
		break;
	}
}

PButtonCtrl CreateButton(U32 CtrlID, structRECT* prect,  U32 FontSize, U32 style, U16 Caption[], PWnd parentWnd)
{
	PButtonCtrl pButton;

	pButton=(PButtonCtrl)CreateOSCtrl(CtrlID,CTRLTYPE_BUTTONCTRL,prect, FontSize,style,parentWnd);
	UstrCpy(pButton->Caption,Caption);

	return pButton;
}

void DestoryButton(PButtonCtrl pButton)
{
	PList pList=GetCtrlList(pButton->parentWnd, pButton->CtrlID);
	DeleteListNode(pList);
	OSMemPut(pCtrlMem, (void*)pButton);
}

void DrawButton(PButtonCtrl pButton)
{
	PDC pdc;
	structRECT rect;
	pdc=GetCtrlParentDC((POS_Ctrl)pButton);
	if(!pdc)
		return;

	FillRect2(pdc, &pButton->ButtonCtrlRect,GRAPH_MODE_NORMAL, COLOR_WHITE);

	CopyRect(&rect, &pButton->ButtonCtrlRect);

	Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
	InflateRect(&rect, -1,-1);
	Draw3DRect2(pdc, &rect, RGB(192,192,192), RGB(0,0,0));
	InflateRect(&rect, -1,-1);
	Draw3DRect2(pdc, &rect, RGB(128,128,128), RGB(0,0,0));
	InflateRect(&rect, -1,-1);

	CopyRect(&pButton->ClientRect, &rect);

	TextOutRect(pdc, &rect, pButton->Caption, TRUE, pButton->FontSize,TEXTOUT_MID_Y|TEXTOUT_MID_X);

	ReleaseCtrlParentDC((POS_Ctrl)pButton);
}

void ButtonOnTchScr(PButtonCtrl pButtonCtrl, int x, int y, U32 tchaction)
{
	POSMSG pMsg;
	structRECT rect;

	CopyRect(&rect, &pButtonCtrl->ClientRect);
	Client2Screen(pButtonCtrl->parentWnd, &rect);
	if(IsInRect(&rect,x,y)){
			PDC pdc;
			structRECT rect;

			CopyRect(&rect, &pButtonCtrl->ButtonCtrlRect);
			pdc=GetCtrlParentDC((POS_Ctrl)pButtonCtrl);

			switch(tchaction){
			case TCHSCR_ACTION_UP:
				Draw3DRect2(pdc,&rect, RGB(192,192,192), RGB(0,0,0));
				InflateRect(&rect, -1, -1);
				Draw3DRect2(pdc,&rect,RGB(192,192,192), RGB(128,128,128));

				pMsg=OSCreateMessage((POS_Ctrl)pButtonCtrl->parentWnd, 
					OSM_BUTTON_CLICK, pButtonCtrl->CtrlID, NULL);
				SendMessage(pMsg);
				break;
			case TCHSCR_ACTION_DOWN:
				Draw3DRect2(pdc,&rect, RGB(0,0,0), RGB(192,192,192));
				InflateRect(&rect, -1, -1);
				Draw3DRect2(pdc,&rect, RGB(128,128,128),RGB(192,192,192));
				break;
			}
			ReleaseCtrlParentDC((POS_Ctrl)pButtonCtrl);
	}
}

void OSButtonCtrlOnMessage(PButtonCtrl pButtonCtrl, POSMSG pMsg)
{
	switch(pMsg->Message){
	case OSM_TOUCH_SCREEN:
		ButtonOnTchScr(pButtonCtrl, (S16)(pMsg->WParam&0xffff), (S16)(pMsg->WParam>>16), pMsg->LParam);
		break;
	case OSM_KEY:
//		onKey(pMsg->WParam,pMsg->LParam);
		break;
	}
}

#endif /*#if USE_MINIGUI==0*/
