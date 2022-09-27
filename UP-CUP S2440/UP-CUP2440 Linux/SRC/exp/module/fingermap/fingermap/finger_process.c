#include "tty.h"

unsigned char UART_RECEIVE_BUFFER[24];


//ͨ��Э�鶨��
unsigned char FP_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  //Э���ͷ
unsigned char FP_Get_Img[6] = {0x01,0x00,0x03,0x01,0x0,0x05};    //���ָ��ͼ��
unsigned char FP_Templete_Num[6] ={0x01,0x00,0x03,0x1D,0x00,0x21 }; //���ģ������
unsigned char FP_Search[11]={0x01,0x0,0x08,0x04,0x01,0x0,0x0,0x03,0xA1,0x0,0xB2}; //����ָ��������Χ0 - 161
unsigned char FP_Search_0_9[11]={0x01,0x0,0x08,0x04,0x01,0x0,0x0,0x0,0x13,0x0,0x21}; //����0-9��ָ��
unsigned char FP_Img_To_Buffer1[7]={0x01,0x0,0x04,0x02,0x01,0x0,0x08}; //��ͼ����뵽BUFFER1
unsigned char FP_Img_To_Buffer2[7]={0x01,0x0,0x04,0x02,0x02,0x0,0x09}; //��ͼ����뵽BUFFER2
unsigned char FP_Reg_Model[6]={0x01,0x0,0x03,0x05,0x0,0x09}; //��BUFFER1��BUFFER2�ϳ�����ģ��
unsigned char FP_Delet_All_Model[6]={0x01,0x0,0x03,0x0d,0x00,0x11};//ɾ��ָ��ģ�������е�ģ��
unsigned char FP_Get_Model_Num[6]={0x01,0x0,0x03,0x1d,0x00,0x21};//ɾ��ָ��ģ�������е�ģ��
volatile unsigned char  FP_Save_Finger[9]={0x01,0x00,0x06,0x06,0x01,0x00,0x0B,0x00,0x19};//��BUFFER1�е��������ŵ�ָ����λ��
//volatile unsigned char FP_Delete_Model[10]={0x01,0x00,0x07,0x0C,0x0,0x0,0x0,0x1,0x0,0x0}; //ɾ��ָ����ģ��

//add by lyj_uptech
void UART_Send_Byte(unsigned char ch)
{
	tty_writeByte(ch);
}
unsigned char  UART_Receive_Byte()
{
	return tty_readByte();
}

//���ָ��ͼ������
void Cmd_Get_Img(void)
{
    unsigned char i;

    for(i=0;i<6;i++) //���Ͱ�ͷ
       UART_Send_Byte(FP_Pack_Head[i]);
    
    for(i=0;i<6;i++) //�������� 0x1d
       UART_Send_Byte(FP_Get_Img[i]);
}


//��ͼ��ת��������������Buffer1��
void Cmd_Img_To_Buffer1(void)
{
 	    unsigned char i;
    
	       for(i=0;i<6;i++)    //���Ͱ�ͷ
	         {
    	       UART_Send_Byte(FP_Pack_Head[i]);   
   		     }
           
   		   for(i=0;i<7;i++)   //�������� ��ͼ��ת���� ������ ����� CHAR_buffer1
             {
      		   UART_Send_Byte(FP_Img_To_Buffer1[i]);
   		     }
}

//��ͼ��ת��������������Buffer2��
void Cmd_Img_To_Buffer2(void)
{
     unsigned char i;
           for(i=0;i<6;i++)    //���Ͱ�ͷ
	         {
    	       UART_Send_Byte(FP_Pack_Head[i]);   
   		     }
           
   		   for(i=0;i<7;i++)   //�������� ��ͼ��ת���� ������ ����� CHAR_buffer1
             {
      		   UART_Send_Byte(FP_Img_To_Buffer2[i]);
   		     }
}

//��BUFFER1 �� BUFFER2 �е�������ϲ���ָ��ģ��
void Cmd_Reg_Model(void)
{
    unsigned char i;    

    for(i=0;i<6;i++) //��ͷ
    {
      UART_Send_Byte(FP_Pack_Head[i]);   
    }

    for(i=0;i<6;i++) //����ϲ�ָ��ģ��
    {
      UART_Send_Byte(FP_Reg_Model[i]);   
    }

}

//ɾ��ָ��ģ���������ָ��ģ��
void Cmd_Delete_All_Model(void)
{
     unsigned char i;    

    for(i=0;i<6;i++) //��ͷ
      UART_Send_Byte(FP_Pack_Head[i]);   

    for(i=0;i<6;i++) //����ϲ�ָ��ģ��
      UART_Send_Byte(FP_Delet_All_Model[i]);   
}

void Cmd_Get_Model_Num(void)
{
     unsigned char i;    

    for(i=0;i<6;i++) //��ͷ
      UART_Send_Byte(FP_Pack_Head[i]);   

    for(i=0;i<6;i++) //����ϲ�ָ��ģ��
      UART_Send_Byte(FP_Get_Model_Num[i]);   
}


//����ȫ���û�162ö
void Cmd_Search_Finger(void)
{
       unsigned char i;	   
	   for(i=0;i<6;i++)   //������������ָ�ƿ�
           {
    	      UART_Send_Byte(FP_Pack_Head[i]);   
   		   }

       for(i=0;i<11;i++)
           {
    	      UART_Send_Byte(FP_Search[i]);   
   		   }


}

//����ȫ���û�162ö
void Cmd_Search_Finger_Admin(void)
{
       unsigned char i;	   
	   for(i=0;i<6;i++)   //������������ָ�ƿ�
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
           
		   for(i=0;i<7;i++)   //����У���
		   	   temp = temp + FP_Save_Finger[i];
			    
		   FP_Save_Finger[7]=(temp & 0x00FF00) >> 8; //���У������
		   FP_Save_Finger[8]= temp & 0x0000FF;
		   
           for(i=0;i<6;i++)    
    	       UART_Send_Byte(FP_Pack_Head[i]);        //���Ͱ�ͷ

           for(i=0;i<9;i++)  
      		   UART_Send_Byte(FP_Save_Finger[i]);      //�������� ��ͼ��ת���� ������ ����� CHAR_buffer1
}


//���շ������ݻ���
void Recevice_Data(unsigned char ucLength)
{
  unsigned char i;

  for (i=0;i<ucLength;i++)
     UART_RECEIVE_BUFFER[i] = UART_Receive_Byte();

}


