#ifndef __RTC_H__
#define __RTC_H__

#include "..\inc\macro.h"

#define RTC_SECOND_CHANGE		1
#define RTC_MINUTE_CHANGE		2
#define RTC_HOUR_CHANGE		3
#define RTC_DAY_CHANGE			4
#define RTC_MONTH_CHANGE		5
#define RTC_YEAR_CHANGE		6

typedef struct{
	U32 year;
	U32 month;
	U32 day;
	U32 date;
	U32 hour;
	U32 minute;
	U32 second;
}structTime, *PstructTime;

typedef struct{
	U32 year;
	U32 month;
	U32 day;
}structDate, *PstructDate;

typedef struct{
	U32 hour;
	U32 minute;
	U32 second;
}structClock, *PstructClock;

void InitRtc(void);
void Get_Rtc(PstructTime);
U8 Rtc_IsTimeChange(U32 whichChange);
void Set_Rtc(PstructTime);
void Rtc_Format(char*  fmtchar, U16* outstr);
void Set_Rtc_Clock(PstructClock time);
void Set_Rtc_Date(PstructDate time);

#endif
