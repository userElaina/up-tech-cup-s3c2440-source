#include"fingermap.h"
#include "finger_process.h"
/**
*���ڹ�˾�ṩ��ͨѶ�У�������Ϣ�Ĵ�С��һ������޷�����ͳһ�Ķ˿����������еĲ�����Ϣ��
*
*������õķ����ǽ���ÿ�βɼ�����rxbuff���ݸô�������
*
*����ɴ��������ж���Ϣ�Ĵ�С����������Ӧ�Ĵ���
*
* @date 2007 08 21
* @auther lyj_uptech@126.com
*
*/

#define  DELAY_TIME  75

/*****************************************************************
 User_enroll()ע���û�
 Command ID:0x21
Э��ͷ	Э�鳤��	�̶��ֶ�	������	ָ�����	Э��β
02	04	        01 02	        21	MM	        02 01 03

******************************************************************/
void User_enroll(unsigned char ucFPIdx)
{
	unsigned char ucDelay_Count;
	unsigned char ucDelay_Exit=0;
	unsigned char Return_Flag = 0;

	unsigned char ucH_user = 0x0;
	unsigned char ucL_user = 0x0;//modify
	ucDelay_Count=DELAY_TIME ;

	Cmd_Get_Model_Num(); 
	Recevice_Data(14);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Ŷϸ���ʷ�
	{
		ucH_user = UART_RECEIVE_BUFFER[10];
		ucL_user = UART_RECEIVE_BUFFER[11] + 1;// MAX = 161
		if(ucL_user >161)
			ucL_user = 161;
	}else{
		printf("device not connect, error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 

	do
	{  
		ucDelay_Count--;
		if(ucDelay_Count==0)
		{
			ucDelay_Exit=1;
			break;
		}          		     	 
     		Cmd_Get_Img(); //��ο�
	 	Recevice_Data(12); //���2�����!�
     	}while ( UART_RECEIVE_BUFFER[9]!=0x00 ); //��ʷ񦵿����
     	if(ucDelay_Exit!=1){
		printf("get one finger\n");
		 
         	Cmd_Img_To_Buffer1(); //���Ͽ��������Buffer1�
	     	Recevice_Data(12);   //���2�����!�
		
		//check if this finger already in store
		Cmd_Search_Finger(); 
	        Recevice_Data(16);   
             	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Ŷϸ���ʷ�i
		{
			printf("this finger already in store ,id = %d\n",UART_RECEIVE_BUFFER[11]);
			return ;
		}
	     	//sleep(1);//modify by lyj_uptech
		 
         	ucDelay_Count=DELAY_TIME ;
	     	do
	     	{  
	          	ucDelay_Count--;
		      	if(ucDelay_Count==0)
	          	{
				ucDelay_Exit=1;
			      	break;
		      	}        
		      	Cmd_Get_Img(); //��ο�
		      	Recevice_Data(12); //���2�����!�			 
	     	}
	     	while( UART_RECEIVE_BUFFER[9]!=0x0 );
	 
	     	if(ucDelay_Exit!=1)
	     	{			
			printf("get another finger\n");
	 	 	Cmd_Img_To_Buffer2(); //���Ͽ��������Buffer2�
             		Recevice_Data(12);   //���2�����!�
	
	         	Cmd_Reg_Model();//�������  
			Recevice_Data(12);
	 
              		if(UART_RECEIVE_BUFFER[9]==0x0)
	          	{
				printf("creat img  ok and saved\n");
				Cmd_Save_Finger(ucH_user,ucL_user);                		         
       		      		Recevice_Data(12);
          		  	Return_Flag=0;
	 		}
	          	else
	          	{ 
		          	Return_Flag=1;
			}
	     	}
	     	else
	     	{
	          Return_Flag=1;
	     	}	
     	}
     	else
	{
	      	Return_Flag=1;
	}
	  
	return ;
}


/*****************************************************************
 User_auth()ָ����֤
 Command ID:0x24
Э��ͷ	Э�鳤��	�̶��ֶ�	������	ָ�����	Э��β
02	04		01 02		24	MM		02 01 03

******************************************************************/
void User_auth(unsigned char ucFPIdx)
{
	unsigned char ucDelay_Count;
	unsigned char ucDelay_Exit=0;
	unsigned char Return_Flag = 0;
	ucDelay_Count=DELAY_TIME ;
		
	do
	{  
	ucDelay_Count--;
	if(ucDelay_Count==0)
	{
	ucDelay_Exit=0x1;
	break;
	} 
        Cmd_Get_Img(); //��˻����   
        Recevice_Data(12); //���2�����!�
	}
	while(UART_RECEIVE_BUFFER[9]!=0x0);
			   
        if(ucDelay_Exit!=0x1) //ȹ����ͳ� {
        Cmd_Img_To_Buffer1(); 
        Recevice_Data(12);   
			    
	Cmd_Search_Finger(); 
	Recevice_Data(16);   
         
	 
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Ŷϸ���ʷ�
	{
		printf("auth ok id =%d match score = %d\n",UART_RECEIVE_BUFFER[11],UART_RECEIVE_BUFFER[13]);
	}else{
		printf("auth error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 


}



/*****************************************************************
 User_erase()ɾ����ָ��ָ�Ƽ�¼
 Command ID:0x22
Э��ͷ	Э�鳤��	�̶��ֶ�	������	ָ�����	Э��β
02	04		01 02		22	MM		02 01 03

******************************************************************/
void User_erase(unsigned char ucFPIdx){
	
}


/*****************************************************************
 User_clear()�������ָ��(ֻ�й���Ա���ܽ��в���,����Ӧ�ó�����֤����Ա���)
 Command ID:0x26
Э��ͷ	Э�鳤��	�̶��ֶ�	������	�����ֶ�	Э��β
02	04		01 02		26	����ֵ		02 01 03

******************************************************************/
void User_clear()
{
	Cmd_Delete_All_Model();
	Recevice_Data(12);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Ŷϸ���ʷ�
	{
		printf("clear ok \n");
	}else{
		printf("clear error \n");
	} 
	
}


/*****************************************************************
 Set_SecuLv()����ָ�Ʊȶ�ʱ���ϸ�ȼ�(00-04)
 Command ID:0x28
Э��ͷ	Э�鳤��	�̶��ֶ�	������	��ȫ����	Э��β
02	04		01 02		28	MM		02 01 03
******************************************************************/
void Set_SecuLv(unsigned char ucSecuLv)
{
}


/*****************************************************************
 Get_FPNum()��õ�ǰ��ע��ָ����
 Command ID:0x27
Э��ͷ	Э�鳤��	�̶��ֶ�	������	Э��β
02	03		01 01		27	02 01 03
******************************************************************/

void Get_FPNum()
{
	
	Cmd_Get_Model_Num(); 
	Recevice_Data(14);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Ŷϸ���ʷ�
	{
		printf("finger num  =%d \n",UART_RECEIVE_BUFFER[11]);
	}else{
		printf("get error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 

}


/*****************************************************************
 FPM_Start()����ָ��ģ��
 Э��ͷ	Э�鳤��	�̶��ֶ�	�����ʣ�4�ֽڣ�	�̶��ֶ�
(28�ֽ�)	Э��β
02	3B	01 39 16 00 00 00 00 50 00 50 00 00 00 0000 00 00 00 64 00 00 00 00 	MM MM MM MM	CC CC CC71 00 00 10 01 00 00 00 00 00 00 00 00 00 01 0010 00 12 00 10 00 00 00 00 00 00 00 00	02 01 03
******************************************************************/
void FPM_Reset()
{
}

/*****************************************************************
 FPM_Reset()��λָ��ģ��
******************************************************************/
void FPM_Start()
{
}

