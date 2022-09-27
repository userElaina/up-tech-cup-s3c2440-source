#include "../inc/macro.h"

#ifndef __TCHSCR_H__
#define __TCHSCR_H__

/////////����������////////
#define TCHSCR_ACTION_NULL			0	
#define TCHSCR_ACTION_CLICK		1	//����������
#define TCHSCR_ACTION_DBCLICK		2	//������˫��
#define TCHSCR_ACTION_DOWN		3	//����������
#define TCHSCR_ACTION_UP			4	//������̧��
#define TCHSCR_ACTION_MOVE		5	//�������ƶ�
#define SUBSRCPND (*(volatile unsigned *)0X4A000018)
#define rADCDLY (*(volatile unsigned *)0x58000008)
#define rADCTSC (*(volatile unsigned *)0x58000004)
#define rADCCON (*(volatile unsigned *)0x58000000)
#define rADCDAT0 (*(volatile unsigned *)0x5800000c)
#define rADCDAT1 (*(volatile unsigned *)0x58000010)
typedef struct {
	int x;
	int y;
	int action;
}tsdata, *Ptsdata;

void TchScr_GetScrXY(int *x, int *y);
//void Touch_Screen_Task(void *Id);		//����������

void TchScr_init(void);
#endif //#ifndef __TCHSCR_H__
