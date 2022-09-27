#include "tty.h"
#include "stdio.h"
#define uchar unsigned char

#define ZX_MSG_ENROLL 0x21  //0x21

#define ZX_MSG_VERIFY 0x24  //0x24

#define ZX_MSG_ERASE 0x22   //0x22

#define ZX_MSG_CLEAR 0x26   //0x26

#define ZX_MSG_SECULV 0x28  //0x28

#define ZX_MSG_GETFPNUM 0x27 //0x27

#define ZX_MSG_START 0x20   //0x20
#define ZX_MSG_RESET 0x20   //0x20
//定义全局变量
static uchar iRslt1,iRslt2,iRslt3,iRstl1;
/*****************************************************************
 User_enroll()注册用户
 Command ID:0x21
 Command:0x01 0x02 0x21 0xff 0x03
******************************************************************/
void User_enroll(unsigned char ucFPIdx);
/*****************************************************************
 User_auth()指纹验证
 Command ID:0x24
 Command:0x01 0x02 0x24 0xff 0x03
******************************************************************/
void User_auth(unsigned char ucFPIdx);
/*****************************************************************
 User_erase()删除指纹记录
 Command ID:0x22
 Command:0x01 0x02 0x22 0xff 0x03
******************************************************************/
void User_erase(unsigned char ucFPIdx);
/*****************************************************************
 User_clear()清除所有指纹(只有管理员才能进行操作)建议应用程序在执行命令之前，验证管理员身份
 Command ID:0x26
 Command:0x01 0x02 0x26 0xff 0x03
******************************************************************/
void User_clear();
/*****************************************************************
 Set_SecuLv()设置指纹比对时的严格等级(00-04)
 Command ID:0x28
协议头	协议长度	固定字段	命令码	安全级别	协议尾
02	04		01 02		28	MM		02 01 03
******************************************************************/
void Set_SecuLv(unsigned char ucSecuLv);
/*****************************************************************
 Get_FPNum()获得当前已注册指纹数
 Command ID:0x27
协议头	协议长度	固定字段	命令码	协议尾
02	03		01 01		27	02 01 03
******************************************************************/

void Get_FPNum();
/*****************************************************************
 FPM_Start()启动指纹模块
 协议头	协议长度	固定字段	波特率（4字节）	固定字段
(28字节)	协议尾
02	3B	01 39 16 00 00 00 00 50 00 50 00 00 00 0000 00 00 00 64 00 00 00 00 	MM MM MM MM	CC CC CC71 00 00 10 01 00 00 00 00 00 00 00 00 00 01 0010 00 12 00 10 00 00 00 00 00 00 00 00	02 01 03
******************************************************************/
void FPM_Start();

/*****************************************************************
 FPM_Reset()复位指纹模块
******************************************************************/
void FPM_Reset();

