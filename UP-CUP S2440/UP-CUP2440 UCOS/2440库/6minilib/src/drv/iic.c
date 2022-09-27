/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2005.2.26
\***************************************************************************/
/***************************************************************************\
    #说明: iic接口驱动程序
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------

	2005-2-26	创建

\***************************************************************************/

#include "../inc/macro.h"
#include "../inc/drv/reg2410.h"
#include "../inc/drv/iic.h"
#include "../inc/sys/lib.h"

#define GPIO_IIC_SCL			(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E14)
#define GPIO_IIC_SDA		(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E15)

#define IIC_READ				1

#define WAIT_IICACK()	do{int i=0; \
			while(!(rIICCON&IICCON_INTPEND)){ i++; hudelay(1);\
			 if(i>1000){ printk("iic ack time out!\n"); break;} }\
			}while(0)

void Set_IIC_mode(int nIICCON, int *poldIICCON)
{	
	if(poldIICCON){
		*poldIICCON=rIICCON;
	}
	rIICCON=nIICCON;
}

void IIC_init(void)
{
	static int time=0;

	if(time!=0)
		return;

	time++;
	
	set_gpio_ctrl (GPIO_IIC_SCL);
	set_gpio_ctrl (GPIO_IIC_SDA);

	//Enable ACK, Prescaler IICCLK=PCLK/512, Enable interrupt, Transmit clock value Tx clock=IICCLK/4
	// If PCLK 50.7MHz, IICCLK = 99KHz, Tx Clock = 25KHz
	rIICCON = IICCON_ACKEN |IICCON_CLK512 | IICCON_INTR | IICCON_CLKPRE(0x3);

	rIICADD  = 0x10;                    //2410 slave address = [7:1]
	rIICSTAT = 0x10;                    //IIC bus data output enable(Rx/Tx)
}

void IIC_MasterTxStart(char data)
{
	int i;

	Disable_Irq(IRQ_IIC);
	
	rIICDS   = data;
	for(i=0;i<10;i++);                  //for setup time until rising edge of IICSCL
	rIICSTAT = IICSTAT_MODE_MT|IICSTAT_START|IICSTAT_OUTEN;	//MasTx,Start 0xf0
//	while(!(IICCON&IICCON_INTPEND));
	WAIT_IICACK();
}

void IIC_MasterTx(char data)
{
	U32 temp;
	int i;

	temp=rIICCON;
	temp  &= (~IICCON_INTPEND);	
	temp |= IICCON_ACKEN;

	rIICDS   = data;
	for(i=0;i<10;i++);                  //for setup time until rising edge of IICSCL
	rIICCON  = temp;	//Resumes IIC operation.
//	while(!(IICCON&IICCON_INTPEND));
	WAIT_IICACK();
}

char IIC_MasterRx(int isACK)
{
	char data;
	U32 temp;

	temp=rIICCON;
	if(isACK){//Resumes IIC operation with ACK
		temp  &= (~IICCON_INTPEND);	
		temp |= IICCON_ACKEN;
	}
	else{	//Resumes IIC operation with NOACK
		temp  &= ~(IICCON_INTPEND|IICCON_ACKEN);
	}

	rIICCON = temp;
//	while(!(IICCON&IICCON_INTPEND));
	WAIT_IICACK();
	data = rIICDS ;


	return data;
}

void IIC_MasterRxStart(char address)
{
	U32 temp;

	Disable_Irq(IRQ_IIC);

	temp=rIICCON;
	temp  &= (~IICCON_INTPEND);	
	temp |= IICCON_ACKEN;

	rIICDS   = address;
	rIICSTAT = IICSTAT_MODE_MR|IICSTAT_START|IICSTAT_OUTEN; //MasRx,Start, 0xb0
	rIICCON = temp;
//	while(!(IICCON&IICCON_INTPEND));
	WAIT_IICACK();
}

void IIC_MasterTxStop(void)
{
	U32 temp;

	temp=rIICCON;
	temp  &= (~IICCON_INTPEND);	
	temp |= IICCON_ACKEN;

	rIICSTAT = IICSTAT_MODE_MT|IICSTAT_OUTEN;                //Stop MasTx condition 0xd0
	rIICCON = temp;
	hudelay(1);                       //Wait until stop condtion is in effect.

	rIICSTAT = IICSTAT_MODE_SR|IICSTAT_OUTEN;                //slaveRx condition
	Enable_Irq(IRQ_IIC);
}

void IIC_MasterRxStop(void)
{
	U32 temp;

	temp=rIICCON;
	temp  &= (~IICCON_INTPEND);	
	temp |= IICCON_ACKEN;

	rIICSTAT = IICSTAT_MODE_MR|IICSTAT_OUTEN;                //Stop MasRx condition 0x90
	rIICCON = temp;
	hudelay(1);                       //Wait until stop condtion is in effect.

	rIICSTAT = IICSTAT_MODE_SR|IICSTAT_OUTEN;                //slaveRx condition
	Enable_Irq(IRQ_IIC);
}

U8 IIC_Read(char devaddr, char address)
{
	U8 data;

	IIC_MasterTxStart(devaddr);
	IIC_MasterTx(address);
	IIC_MasterRxStart(devaddr|IIC_READ);

	data=IIC_MasterRx(FALSE);

	IIC_MasterRxStop();

	return data;
}

void IIC_Write(char devaddr, char address, unsigned char data)
{
	IIC_MasterTxStart(devaddr);
	IIC_MasterTx(address);
	IIC_MasterTx(data);
	IIC_MasterTxStop();
}

void IIC_ReadSerial(char devaddr, char address, unsigned char* pdata, int n)
{
	if (n <=0 )
		return;
	n--;

	IIC_MasterTxStart(devaddr);
	IIC_MasterTx(address);
	IIC_MasterRxStart(devaddr|IIC_READ);

	for(;n>0;n--){
		*pdata=IIC_MasterRx(TRUE);
		pdata++;
	}

	*pdata=IIC_MasterRx(FALSE);

	IIC_MasterRxStop();
}

void IIC_WriteSerial(char devaddr, char address, unsigned char* pdata, int n)
{
	if (n <=0 )
		return;

	IIC_MasterTxStart(devaddr);
	IIC_MasterTx(address);

	for(;n>0;n--){
		IIC_MasterTx(*pdata);
		pdata++;
	}

//	*pdata=IIC_MasterRx(FALSE);

	IIC_MasterTxStop();
}

void IIC_Send(char devaddr, const char* pdata, int n)
{
	if (n <=0 )
		return;

	IIC_MasterTxStart(devaddr<<1);
	
	for(;n>0;n--){
		IIC_MasterTx(*pdata);
		pdata++;
	}

	IIC_MasterTxStop();
}


