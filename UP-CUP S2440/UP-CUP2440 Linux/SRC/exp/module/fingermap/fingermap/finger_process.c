#include "tty.h"

unsigned char UART_RECEIVE_BUFFER[24];


//通信协议定义
unsigned char FP_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  //协议包头
unsigned char FP_Get_Img[6] = {0x01,0x00,0x03,0x01,0x0,0x05};    //获得指纹图像
unsigned char FP_Templete_Num[6] ={0x01,0x00,0x03,0x1D,0x00,0x21 }; //获得模版总数
unsigned char FP_Search[11]={0x01,0x0,0x08,0x04,0x01,0x0,0x0,0x03,0xA1,0x0,0xB2}; //搜索指纹搜索范围0 - 161
unsigned char FP_Search_0_9[11]={0x01,0x0,0x08,0x04,0x01,0x0,0x0,0x0,0x13,0x0,0x21}; //搜索0-9号指纹
unsigned char FP_Img_To_Buffer1[7]={0x01,0x0,0x04,0x02,0x01,0x0,0x08}; //将图像放入到BUFFER1
unsigned char FP_Img_To_Buffer2[7]={0x01,0x0,0x04,0x02,0x02,0x0,0x09}; //将图像放入到BUFFER2
unsigned char FP_Reg_Model[6]={0x01,0x0,0x03,0x05,0x0,0x09}; //将BUFFER1跟BUFFER2合成特征模版
unsigned char FP_Delet_All_Model[6]={0x01,0x0,0x03,0x0d,0x00,0x11};//删除指纹模块里所有的模版
unsigned char FP_Get_Model_Num[6]={0x01,0x0,0x03,0x1d,0x00,0x21};//删除指纹模块里所有的模版
volatile unsigned char  FP_Save_Finger[9]={0x01,0x00,0x06,0x06,0x01,0x00,0x0B,0x00,0x19};//将BUFFER1中的特征码存放到指定的位置
//volatile unsigned char FP_Delete_Model[10]={0x01,0x00,0x07,0x0C,0x0,0x0,0x0,0x1,0x0,0x0}; //删除指定的模版

//add by lyj_uptech
void UART_Send_Byte(unsigned char ch)
{
	tty_writeByte(ch);
}
unsigned char  UART_Receive_Byte()
{
	return tty_readByte();
}

//获得指纹图像命令
void Cmd_Get_Img(void)
{
    unsigned char i;

    for(i=0;i<6;i++) //发送包头
       UART_Send_Byte(FP_Pack_Head[i]);
    
    for(i=0;i<6;i++) //发送命令 0x1d
       UART_Send_Byte(FP_Get_Img[i]);
}


//讲图像转换成特征码存放在Buffer1中
void Cmd_Img_To_Buffer1(void)
{
 	    unsigned char i;
    
	       for(i=0;i<6;i++)    //发送包头
	         {
    	       UART_Send_Byte(FP_Pack_Head[i]);   
   		     }
           
   		   for(i=0;i<7;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
             {
      		   UART_Send_Byte(FP_Img_To_Buffer1[i]);
   		     }
}

//将图像转换成特征码存放在Buffer2中
void Cmd_Img_To_Buffer2(void)
{
     unsigned char i;
           for(i=0;i<6;i++)    //发送包头
	         {
    	       UART_Send_Byte(FP_Pack_Head[i]);   
   		     }
           
   		   for(i=0;i<7;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
             {
      		   UART_Send_Byte(FP_Img_To_Buffer2[i]);
   		     }
}

//将BUFFER1 跟 BUFFER2 中的特征码合并成指纹模版
void Cmd_Reg_Model(void)
{
    unsigned char i;    

    for(i=0;i<6;i++) //包头
    {
      UART_Send_Byte(FP_Pack_Head[i]);   
    }

    for(i=0;i<6;i++) //命令合并指纹模版
    {
      UART_Send_Byte(FP_Reg_Model[i]);   
    }

}

//删除指纹模块里的所有指纹模版
void Cmd_Delete_All_Model(void)
{
     unsigned char i;    

    for(i=0;i<6;i++) //包头
      UART_Send_Byte(FP_Pack_Head[i]);   

    for(i=0;i<6;i++) //命令合并指纹模版
      UART_Send_Byte(FP_Delet_All_Model[i]);   
}

void Cmd_Get_Model_Num(void)
{
     unsigned char i;    

    for(i=0;i<6;i++) //包头
      UART_Send_Byte(FP_Pack_Head[i]);   

    for(i=0;i<6;i++) //命令合并指纹模版
      UART_Send_Byte(FP_Get_Model_Num[i]);   
}


//搜索全部用户162枚
void Cmd_Search_Finger(void)
{
       unsigned char i;	   
	   for(i=0;i<6;i++)   //发送命令搜索指纹库
           {
    	      UART_Send_Byte(FP_Pack_Head[i]);   
   		   }

       for(i=0;i<11;i++)
           {
    	      UART_Send_Byte(FP_Search[i]);   
   		   }


}

//搜索全部用户162枚
void Cmd_Search_Finger_Admin(void)
{
       unsigned char i;	   
	   for(i=0;i<6;i++)   //发送命令搜索指纹库
           {
    	      UART_Send_Byte(FP_Pack_Head[i]);   
   		   }

       for(i=0;i<11;i++)
           {
    	      UART_Send_Byte(FP_Search_0_9[i]);   
   		   }


}

void Cmd_Save_Finger( unsigned char ucH_Char,unsigned char ucL_Char )
{
           unsigned long temp = 0;
		   unsigned char i;
           FP_Save_Finger[5] = ucH_Char;
           FP_Save_Finger[6] = ucL_Char;
           
		   for(i=0;i<7;i++)   //计算校验和
		   	   temp = temp + FP_Save_Finger[i];
			    
		   FP_Save_Finger[7]=(temp & 0x00FF00) >> 8; //存放校验数据
		   FP_Save_Finger[8]= temp & 0x0000FF;
		   
           for(i=0;i<6;i++)    
    	       UART_Send_Byte(FP_Pack_Head[i]);        //发送包头

           for(i=0;i<9;i++)  
      		   UART_Send_Byte(FP_Save_Finger[i]);      //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
}


//接收反馈数据缓冲
void Recevice_Data(unsigned char ucLength)
{
  unsigned char i;

  for (i=0;i<ucLength;i++)
     UART_RECEIVE_BUFFER[i] = UART_Receive_Byte();

}


