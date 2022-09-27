#include "../ucos-ii/includes.h"

#include "../inc/sys/lib.h"
#include "../inc/sys/can.h"
#include "../inc/drv/reg2410.h"
#include "../ucos-ii/Uhal/Uhal.h"
#include "../ucos-ii/Uhal/isr.h"

//#define DPRINTF		printf
#define DPRINTF		printfNULL


/********************** Candata *********************************/
#define CAN2410_Isr(n)		(CAN0_OFFSET+n)	//串口中断号

static int CAN_Base[]={CAN0BASE, CAN1BASE};

//#define can_outb(ndev, offset, ch)	_outb(CAN_Base[ndev] + (offset,) (ch))
#define can_outw(ndev, offset, ch)	do{int i; _outw(CAN_Base[ndev] + (offset), (ch)); for(i=0;i<10;i++);}while(0)
//#define can_outl(ndev, offset, ch)	_outl(CAN_Base[ndev] + (offset), (ch))

#define FIRST_MSG				1
#define LAST_MSG				32

//#define can_inb(ndev, offset)	_inb(CAN_Base[ndev] + offset)
__inline unsigned short can_inw(int ndev, int offset)
{
	int i;
	unsigned short temp;

	temp=_inw(CAN_Base[ndev] + offset);
	for(i=0;i<10;i++);
	temp=_inw(CAN_Base[ndev] + offset);
	for(i=0;i<10;i++);
	return temp;
}
//#define can_inl(ndev, offset)	_inl(CAN_Base[ndev] + offset)


/************************************************************\
*	interface x busy检查										*
*	参数: ndev为设备									*
*			n为IF 接口									*
\************************************************************/
__inline int can_IFx_busycheck(int ndev, int n)
{

	int i=0;
	unsigned short comreg = 0;
	int addr[]={IF1ComR, IF2ComR};

	comreg = can_inw( ndev, addr[n-1]);
	while ( (comreg & IFXCom_Busy) && (i<=10))
	{
		udelay(100); //100 microseconds
		i++;
		comreg = can_inw( ndev, addr[n-1]);
	}
	if (i>=10){
		printk("Error Busy-Bit stays set\n");
		return FAIL;
	}

	return OK;
}

/************************************************************\
*	等待发送结束										*
*	参数: ndev为设备									*
*			nMsg为第nMsg接口1-32						*
\************************************************************/
__inline void wait_for_Sendend(int ndev, int nMsg)
{
	U32 txrqst;

	do{
		txrqst=can_inw(ndev, TxRqst2);
		txrqst<<=16;
		txrqst|=can_inw(ndev, TxRqst1);
	}while(txrqst & (1<<(nMsg-1)));

}


/************************************************************\
*	设置CAN2410 CAN总线波特率						*
*	参数: bandrate为所设置的波特率				*
*			IsBackNormal为是否要返回Normal模式		*
\************************************************************/
__inline void CAN2410_SetBandRate(int ndev, CanBandRate bandrate)
{
	//
	// Bit rate calculations.
	//
	//Input clock fre=8MHz
	// In this case, we'll use a speed of 125 kbit/s, 250 kbit/s, 500 kbit/s, 1Mbit/s.
	// one bit will be 1+4+3 = 8 quanta in length.
	//Set Tseg1=4, Tseg2=3, SJW=1
	//
	// 8MHz: setting the prescaler (BRP+1) to 1 => 1 Mbit/s.
	// 4MHz: setting the prescaler (BRP+1) to 2 => 500 kbit/s.
	// 2MHz: setting the prescaler (BRP+1) to 4 => 250 kbit/s.
	// 1MHz: setting the prescaler (BRP+1) to 8 => 125 kbit/s.

	int brp=0;

	switch(bandrate){
	case BandRate_125kbps:
		brp=8;
		break;
	case BandRate_250kbps:
		brp=4;
		break;
	case BandRate_500kbps:
		brp=2;
		break;
	case BandRate_1Mbps:
		brp=1;
		break;
	}
	brp--;

	// Go into configuration mode
	can_outw(ndev, CANControl, CANControl_CCE|CANControl_Init);

	can_outw(ndev, CANBRPExt, 0);
	can_outw(ndev, CANBitTimReg, 
		CANBitTim_BRP(brp)|CANBitTim_SWJ(0)|CANBitTim_TSeg1(3)|CANBitTim_TSeg2(2));

	// Go back into normal mode
	can_outw(ndev, CANControl, 0);
}

/***********************************************************************************\
								发送数据
	参数:
		ndev，设备号
		data，发送数据

	Note: 使用interface 1		利用两个缓冲区乒乓发送
\***********************************************************************************/
__inline int CAN2410_canWrite(int ndev, PCanData data)
{
#undef readMaskCM
#undef writeMaskCM
#undef writeSendMskCM
#define readMaskCM		(IFXMask_Control | IFXMask_Arb | IFXMask_DataA | IFXMask_DataB)
#define writeMaskCM		(IFXMask_Control | IFXMask_Arb | IFXMask_DataA | IFXMask_DataB|IFXMask_WR)
//#define writeSendMskCM	(IFXMask_Control |IFXMask_Arb | IFXMask_DataA | IFXMask_DataB| IFXMask_WR | IFXMask_TxRqst)

	static int nMsg=0;

	unsigned short *pdata;

//	int temp;

//	temp=can_inw(1, MsgVal1);
//	temp=can_inw(1, MsgVal2);

	wait_for_Sendend(ndev, nMsg+1);

	//load Message Object in IF1
	can_IFx_busycheck(ndev, 1);
	can_outw(ndev, IF1ComM, readMaskCM);

	//set interface into nMsg
	can_outw(ndev, IF1ComR, IFXCom_Busy|IFXCom_Msg(nMsg+1));
	can_IFx_busycheck(ndev, 1);

	//setting Message Valid Bit to zero
	can_outw(ndev, IF1Arb2, 0);

	can_outw(ndev, IF1ComM, writeMaskCM);
	can_outw(ndev, IF1ComR, IFXCom_Busy|IFXCom_Msg(nMsg+1));

	can_IFx_busycheck(ndev, 1);
   	//Configuring MO
//	tempreg = can_inw( ndev, IF2ComM);
	//remote enable?
	//define Command Mask
	if(data->rxRTR)
		can_outw(ndev, IF1Mctrl, IFXM_EoB | IFXM_RmtEn | IFXM_NewDat
				| IFXM_TxRqst | IFXM_DLCMask);
	else
		can_outw(ndev, IF1Mctrl, IFXM_EoB | IFXM_NewDat
				| IFXM_TxRqst | IFXM_SETDLC(data->dlc));

	//set Arbitration Bits
	if (data->IsExt){
		can_outw(ndev, IF1Arb1, (U16)(data->id));
		can_outw(ndev, IF1Arb2, IFXARB2_XTD | IFXARB2_MVAL | IFXARB2_TR |IFXARB2_SETEID(data->id>>16));
	}
	else{
		can_outw(ndev, IF1Arb2, IFXARB2_MVAL | IFXARB2_TR | IFXARB2_SETID(data->id<<2));
		can_outw(ndev, IF1Arb1, 0);
	}
	//write Data
	if (data->dlc>0){

		pdata = (unsigned short*)(&data->data[0]);
		can_outw(ndev, IF1DataA1, *pdata);
		pdata = (unsigned short*)(&data->data[2]);
		can_outw(ndev, IF1DataA2, *pdata);
		pdata = (unsigned short*)(&data->data[4]);
		can_outw(ndev, IF1DataB1, *pdata);
		pdata = (unsigned short*)(&data->data[6]);
		can_outw(ndev, IF1DataB2, *pdata);
	}

//	can_outw(ndev, IF1ComM, writeSendMskCM);
	can_outw(ndev, IF1ComM, writeMaskCM);
	can_outw(ndev, IF1ComR, IFXCom_Busy|IFXCom_Msg(nMsg+1));

	nMsg++;
	nMsg&=1;

	return OK;
}

__inline int CAN2410_canRead(int ndev, PCanData data)
{

#undef msgLstReadMaskCM
#undef msgLstWriteMaskCM
#undef readMaskCM
#define msgLstReadMaskCM	(IFXMask_Control)
#define msgLstWriteMaskCM	(IFXMask_Control | IFXMask_WR)
#define readMaskCM			(IFXMask_Arb|IFXMask_Control| IFXMask_ClrIntPnd |IFXMask_TxRqst| IFXMask_DataA | IFXMask_DataB)

	unsigned int nMsg, val;

	nMsg=can_inw(ndev, CANIntReg);
	DPRINTF("nMsg=%d\n", nMsg);
	if(nMsg<1 || nMsg>32)
		return FAIL;

	//Message Lost Check
	can_IFx_busycheck(ndev, 2);

	can_outw(ndev, IF2ComM, msgLstReadMaskCM);
	//set interface into nMsg
	can_outw(ndev, IF2ComR, IFXCom_Busy|IFXCom_Msg(nMsg));

	can_IFx_busycheck(ndev, 2);

	val = can_inw(ndev, IF2Mctrl);

	if (val & IFXM_MsgLst){
		printk("Chip lost a can message\n");

	       //Reset Message Lost Bit
		val &= ~IFXM_MsgLst;
		can_outw(ndev, IF2Mctrl, val);
		can_outw(ndev, IF2ComM, msgLstWriteMaskCM);
		//set interface into nMsg
		can_outw(ndev, IF2ComR, IFXCom_Busy|IFXCom_Msg(nMsg));

		can_IFx_busycheck(ndev, 2);
	}

	//transfer Message Object to IF2 Buffer
	can_outw(ndev, IF2ComM, readMaskCM);
	//set interface into nMsg
	can_outw(ndev, IF2ComR, IFXCom_Busy|IFXCom_Msg(nMsg));
	can_IFx_busycheck(ndev, 2);

	val=can_inw(ndev, IF2Arb2);
	if ( val& IFXARB2_TR)
		return FAIL;

	DPRINTF("Arb2=0x%x\n", val);
	
	if (val & IFXARB2_XTD){	//Externd frame
		val=(can_inw(ndev, IF2Arb2) & IFXARB2_EIDMask)<<16;
		val|=can_inw(ndev, IF2Arb1);
		data->id= val;
	}
	else{
		data->id=(can_inw(ndev, IF2Arb2)&IFXARB2_IDMask)>>2;
	}

	val=can_inw(ndev, IF2Mctrl);
	DPRINTF("Message contrl=0x%x\n", val&(~0xf));
	data->dlc=IFXM_GETDLC(val);

	//remote frame ?
/*	if(data->rxRTR)
		can_outw(ndev, IF1Mctrl, IFXM_EoB | IFXM_TxIE | IFXM_RmtEn | IFXM_NewDat
				| IFXM_TxRqst | IFXM_DLCMask);
	else
		can_outw(ndev, IF1Mctrl, IFXM_EoB | IFXM_TxIE | IFXM_NewDat
				| IFXM_TxRqst | IFXM_DLCMask);
*/

	//??
	can_outw(ndev, IF2Mctrl, val&(~IFXM_IntPnd));

	if (data->dlc==0){
		return OK;
	}

	val = can_inw(ndev, IF2DataA1);
	data->data[0]=val & 0xff;
	data->data[1]=(val>>8) & 0xff;
	val = can_inw(ndev, IF2DataA2);
	data->data[2]=val & 0xff;
	data->data[3]=(val>>8) & 0xff;
	val = can_inw(ndev, IF2DataB1);
	data->data[4]=val & 0xff;
	data->data[5]=(val>>8) & 0xff;
	val = can_inw(ndev, IF2DataB2);
	data->data[6]=val & 0xff;
	data->data[7]=(val>>8) & 0xff;

	return OK;
}


/*********************************************************************\
	2410CAN Mask设置函数
	参数:
		ndev，设备号
		nMsg，Message号
		mask，
	使用interface 1
\*********************************************************************/
static void Can2410_SetMask(int ndev, int nMsg, U32 mask, int ext, int mdir)
{
#undef readMaskCM
#undef writeMaskCM
#define readMaskCM	(IFXMask_Control| IFXMask_Arb| IFXMask_Mask)
#define writeMaskCM	(IFXMask_Control| IFXMask_Arb| IFXMask_Mask|IFXMask_WR)

	//save the old Message number
	U32 tempreg;

	can_IFx_busycheck(ndev, 1);
	//load Message Object in IF1
	can_outw(ndev, IF1ComM, readMaskCM);
	//set interface into nMsg
	can_outw(ndev, IF1ComR, IFXCom_Busy|IFXCom_Msg(nMsg));

	can_IFx_busycheck(ndev, 1);

	//setting Message Valid Bit to zero
/*	can_outw(ndev, IF1Arb2, 0);

	can_outw(ndev, IF1ComM, writeMaskCM);
	can_outw(ndev, IF1ComR, IFXCom_Busy|IFXCom_Msg(nMsg));

	can_IFx_busycheck(ndev, 1);*/
	//setting UMask, MsgVal and Mask Register
	//writing acceptance mask for extended or standart mode
	if (ext){
		can_outw(ndev, IF1Mask2, (mask>>16 & 0x1FFF) | IFXMSK2_MXTD | IFXMSK2_SETMDIR(mdir));
		can_outw(ndev, IF1Mask1, (mask & 0xFFFF));
	}
	else{
		can_outw(ndev, IF1Mask2, ((mask<<2) & 0x1FFC) |IFXMSK2_SETMDIR(mdir));
		can_outw(ndev, IF1Mask1, 0);
	}

	can_outw(ndev, IF1Mctrl, IFXM_RxIE|IFXM_EoB|IFXM_UMask);
	
	//seting Message Valid Bit to one
	can_outw(ndev, IF1Arb2, IFXARB2_MVAL);

	can_outw(ndev, IF1ComM, writeMaskCM);
	can_outw(ndev, IF1ComR, IFXCom_Busy|IFXCom_Msg(nMsg));
}

/*********************************************************************\
	CAN设备初始化函数
	参数:	
		ndev，设备号
		bandrate，CAN波特率
		LoopBack，是否是LoopBack模式
		pfilter，CAN过滤器结构体指针，
					如果为NULL，则接受所有数据

	设置32个Message中,1-2为发送缓冲区，3-32为接受缓冲区
\*********************************************************************/
__inline int init_CAN2410(int ndev, CanBandRate bandrate, BOOL LoopBack, PCanFilter pfilter)
{
	unsigned char i;

	// disable can moduels
	can_outw(ndev, CANEnable, 0);
	udelay(100);
	// Enable can moduels
	can_outw(ndev, CANEnable, CANEnable_En);

	Enable2410_PLL2();
	CAN2410_SetBandRate(ndev, bandrate);

	//set message number 1,2 receive non
	i=FIRST_MSG;
	Can2410_SetMask(ndev, i++, 0x1fffffff, 1, 1);
	Can2410_SetMask(ndev, i++, 0x1fffffff, 1, 1);
	if(!pfilter){
		for(;i<=LAST_MSG;i++){
			Can2410_SetMask(ndev, i, 0, 1, 0);
		}
	}
	else{
		// set all filters to same = pfilter->Filter:
		for(;i<=LAST_MSG;i++){
			Can2410_SetMask(ndev, i, pfilter->Filter, pfilter->IsExt, 0);
		}
	}

   // Clear Status to get better starting
/*	i=32;
	while (i --){
		int val = can_inw(ndev, CANStatus);
		can_outw(ndev, CANStatus, 0);
		val = can_inw(ndev, CANStatus);
		if(val != 0) {
			printk("Initialization ERRROR: Status = 0x%x\n", val);
		}
		else
			break;
	}
	
	if(i == 0) {
		printk("CAN bus Initialization error. Can't clear can Status reg\n");
		return FAIL;
	}*/

	//Open Interrupt
//	can_outw(ndev, CANControl, CANControl_IE|CANControl_DAR);
	can_outw(ndev, CANControl, CANControl_IE|CANControl_EIE/*|CANControl_SIE*/);

	return OK;
}

/*********************************************************************\
	CAN设备查询函数
	参数:	
		ndev，设备号
	返回CANPOLL_Rev/CANPOLL_SendEnd
\*********************************************************************/
static int CAN2410_CanPoll(int ndev)
{
	int ret=0, status, intr;

	intr=can_inw(ndev, CANIntReg);
	DPRINTF("Poll: intr=%d\n", intr);
	status=can_inw(ndev, CANStatus);

	if(intr>0 && intr<33 && (status & CANStatus_TxOk)){	//message object interrupt
		can_IFx_busycheck(ndev, 1);
		can_outw(ndev, IF1ComM, IFXMask_ClrIntPnd);
		//clear interrupt
		can_outw(ndev, IF1ComR, IFXCom_Busy|intr);
	}

	if(status & CANStatus_RxOk){
		can_outw(ndev, CANStatus, status&(~CANStatus_RxOk));
		ret |=CANPOLL_Rev;
	}

	if(status & CANStatus_TxOk){
		can_outw(ndev, CANStatus, status&(~CANStatus_TxOk));
		ret |=CANPOLL_SendEnd;
	}

	return ret;
}

/*int CAN2410_canFlushInput()
{
	return 0;
}*/

#define DefCan2410N(n)	\
	static int s3c2410_can##n##_init(CanBandRate baud, BOOL LoopBack, PCanFilter pfilter)\
	{return init_CAN2410(n,baud, LoopBack, pfilter);}\
	static int s3c2410_can##n##_read(PCanData data){return CAN2410_canRead(n, data);}\
	static int s3c2410_can##n##_write(PCanData data){return CAN2410_canWrite(n, data);}\
	static int s3c2410_can##n##_poll(void){return CAN2410_CanPoll(n);}

/* export can driver */
#define ExportCan2410N(n)	can_driver_t s3c2410_can##n##_driver = {	\
								s3c2410_can##n##_init,	\
								NULL,/*CAN2410_CanOpen*/	\
								NULL,/*CAN2410_CanClose*/	\
								s3c2410_can##n##_read,	\
								s3c2410_can##n##_write,	\
								s3c2410_can##n##_poll,	\
								NULL,/*CAN2410_canFlushInput*/	\
								NULL,/*CAN2410_canFlushoutput*/	\
								CAN2410_Isr(n)}

DefCan2410N(0)
DefCan2410N(1)

ExportCan2410N(0);
ExportCan2410N(1);


/*can_driver_t can2410_driver = {
	init_CAN2410,
	NULL,	//CAN2410_CanOpen
	NULL,	//CAN2410_CanClose
	CAN2410_canRead,
	CAN2410_canWrite,
	CAN2410_CanPoll,
	NULL,	//CAN2410_canFlushInput
	NULL
};*/

