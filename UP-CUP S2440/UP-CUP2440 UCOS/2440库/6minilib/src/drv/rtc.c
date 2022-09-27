#include	"../ucos-ii/includes.h"               /* uC/OS interface */
#include "inc/drv/rtc.h"
#include "inc/drv/reg2410.h"

OS_EVENT *Rtc_Rw_Sem;                   //Rtc读写控制权旗语
//and you can use it as folloeing:
//    Rtc_Rw_Sem=OSSemCreate(1);        //创建Rtc读写控制权旗语,初值为1满足互斥条件//
//    OSSemPend(Rtc_Rw_Sem,0,&err);
//    OSSemPost(Rtc_Rw_Sem);

void InitRtc()
{
	Rtc_Rw_Sem=OSSemCreate(1);        //创建Rtc读写控制权旗语,初值为1满足互斥条件//
}

void Get_Rtc(PstructTime time)
{
	unsigned char err;
	OSSemPend(Rtc_Rw_Sem,0,&err);
	
	rRTCCON = 0x01;         // R/W enable, 1/32768, Normal(merge), No reset
	time->year=rBCDYEAR;
	time->month=rBCDMON;
	time->date=rBCDDATE;
	time->day=rBCDDAY;
	time->hour=rBCDHOUR;
	time->minute=rBCDMIN;
	time->second=rBCDSEC;
	rRTCCON = 0x00;       // R/W disable(for power consumption), 1/32768, Normal(merge), No reset    
	
	OSSemPost(Rtc_Rw_Sem);
}

U8 Rtc_IsTimeChange(U32 whichChange)
{
	unsigned char err;
	static U32 second=0xff,minute=0xff,hour=0xff,day=0xff;
	U8 retvalue=FALSE;

	OSSemPend(Rtc_Rw_Sem,0,&err);
	rRTCCON = 0x01;         // R/W enable, 1/32768, Normal(merge), No reset
	switch(whichChange){
	case RTC_SECOND_CHANGE:
		if(rBCDSEC!=second){
			retvalue=TRUE;
			second=rBCDSEC;
		}
		break;
	case RTC_MINUTE_CHANGE:
		if(rBCDMIN==minute){
			retvalue=TRUE;
			minute=rBCDMIN;
		}
		break;
	case RTC_HOUR_CHANGE:
		if(rBCDHOUR==hour){
			retvalue=TRUE;
			hour=rBCDHOUR;
		}
		break;
	case RTC_DAY_CHANGE:{
			retvalue=TRUE;
			day=rBCDDAY;
		}
		break;
	}
	rRTCCON = 0x00;       // R/W disable(for power consumption), 1/32768, Normal(merge), No reset    
	OSSemPost(Rtc_Rw_Sem);

	return retvalue;
}

void Set_Rtc(PstructTime time)
{
	unsigned char err;
	OSSemPend(Rtc_Rw_Sem,0,&err);

	rRTCCON = 0x01;         // R/W enable, 1/32768, Normal(merge), No reset
	rBCDYEAR=time->year;
	rBCDMON=time->month;
	rBCDDAY=time->day;
	rBCDHOUR=time->hour;
	rBCDMIN=time->minute;
	rBCDSEC=time->second;
	rRTCCON = 0x00;       // R/W disable(for power consumption), 1/32768, Normal(merge), No reset    

	OSSemPost(Rtc_Rw_Sem);
}

void GetBCDTime2Str(U16* fmtchar,U32 bcdata)
{
	*fmtchar=(bcdata>>4)+'0';
	fmtchar++;
	*fmtchar=(bcdata&0xf)+'0';
}

void Rtc_Format(char*  fmtchar, U16* outstr)
{
	structTime time;
	Get_Rtc(&time);
	while(*fmtchar){
		if((*fmtchar)==((U16)'%')){
			switch(*(fmtchar+1)){
			case 'Y':
				GetBCDTime2Str(outstr, time.year);
				fmtchar+=2;
				outstr+=2;
				break;
			case 'M':	//month
				GetBCDTime2Str(outstr, time.month);
				fmtchar+=2;
				outstr+=2;
				break;
			case 'D':
				GetBCDTime2Str(outstr, time.day);
				fmtchar+=2;
				outstr+=2;
				break;
			case 'H':
				GetBCDTime2Str(outstr, time.hour);
				fmtchar+=2;
				outstr+=2;
				break;
			case 'I':	//minute
				GetBCDTime2Str(outstr, time.minute);
				fmtchar+=2;
				outstr+=2;
				break;
			case 'S':
				GetBCDTime2Str(outstr, time.second);
				fmtchar+=2;
				outstr+=2;
				break;
			case '%':
				*outstr='%';
				fmtchar++;
				outstr++;
			case NULL:
				return;
			}
		}
		else{
			*outstr=*fmtchar;
			fmtchar++;
			outstr++;
		}
	}
	*outstr=0;
}

void Set_Rtc_Clock(PstructClock time)
{
	unsigned char err;
	OSSemPend(Rtc_Rw_Sem,0,&err);

	rRTCCON = 0x01;         // R/W enable, 1/32768, Normal(merge), No reset
	rBCDHOUR=time->hour;
	rBCDMIN=time->minute;
	rBCDSEC=time->second;
	rRTCCON = 0x00;       // R/W disable(for power consumption), 1/32768, Normal(merge), No reset    

	OSSemPost(Rtc_Rw_Sem);
}

void Set_Rtc_Date(PstructDate time)
{
	unsigned char err;
	OSSemPend(Rtc_Rw_Sem,0,&err);

	rRTCCON = 0x01;         // R/W enable, 1/32768, Normal(merge), No reset
	rBCDYEAR=time->year;
	rBCDMON=time->month;
	rBCDDAY=time->day;
	rRTCCON = 0x00;       // R/W disable(for power consumption), 1/32768, Normal(merge), No reset    

	OSSemPost(Rtc_Rw_Sem);
}
