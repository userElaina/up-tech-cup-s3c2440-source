/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #˵��: s3c44b0 AD�ӿڳ���
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------����--------------------------------------
	2004-8-12	����

\***************************************************************************/


/* ���Ļ��Ķ���*/



#include "../inc/sys/lib.h"
#include "../inc/drv/AD.h"

#define ADCCON_FLAG		(0x1<<15)

#define ADCCON_ENABLE_START_BYREAD	(0x1<<1)


void init_ADdevice()
{//��ʼ��
	rADCCON=(PRSCVL|ADCCON_ENABLE_START|STDBM|PRSCEN);
}

int GetADresult(int channel)
{   
	rADCCON=ADCCON_ENABLE_START_BYREAD|(channel<<3)|PRSCEN|PRSCVL;
	hudelay(10);
	while(!(rADCCON&ADCCON_FLAG));//ת������
	return (0x3ff&rADCDAT0);//���ز���ֵ	
}





/*int AD_GetResult(int channel)
{
	rADCDIRCR &= ~ADCDIRCR_AIOSTOP;	//start adc
	rADCDIRCR = ADCDIRCR_ADIN(channel);
	udelay(500);
//	while(!(rADCDIRCR&ADCDIRCR_AIOSTOP));

	return rADCDIRDATA;
}*/

void AD_test(void)
{
	int i,j;

	init_ADdevice();
	printf("\n");
	for(;;){
		float ret;
		printf("\r");
		for(i=0;i<5;i++){
			for(j=0;j<=1;j++)
			{
			ret=GetADresult(i);
			}
			ret=ret*5.0/1024;
			printf("AD%d=%f\t",i, ret);
			OSTimeDly(10);
		}
		OSTimeDly(500);
	}
}
