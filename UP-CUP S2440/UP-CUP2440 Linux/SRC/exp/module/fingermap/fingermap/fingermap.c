#include"fingermap.h"
#include "finger_process.h"
/**
*由于公司提供的通讯中，返回信息的大小不一，因此无法采用统一的端口来处理所有的参数信息；
*
*这里采用的方法是将，每次采集到的rxbuff传递该处理函数，
*
*最后由处理函数来判断信息的大小，并做出相应的处理
*
* @date 2007 08 21
* @auther lyj_uptech@126.com
*
*/

#define  DELAY_TIME  75

/*****************************************************************
 User_enroll()注册用户
 Command ID:0x21
协议头	协议长度	固定字段	命令码	指纹序号	协议尾
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
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //哦细嗡耸否?
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
     		Cmd_Get_Img(); //获肝肯
	 	Recevice_Data(12); //接?2缚靠?!?
     	}while ( UART_RECEIVE_BUFFER[9]!=0x00 ); //检史瘭悼戳课
     	if(ucDelay_Exit!=1){
		printf("get one finger\n");
		 
         	Cmd_Img_To_Buffer1(); //将肯炕怀韶麈訠uffer1?
	     	Recevice_Data(12);   //接?2缚靠?!?
		
		//check if this finger already in store
		Cmd_Search_Finger(); 
	        Recevice_Data(16);   
             	if( UART_RECEIVE_BUFFER[9]==0x0 )  //哦细嗡耸否
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
		      	Cmd_Get_Img(); //获肝肯
		      	Recevice_Data(12); //接?2缚靠?!?			 
	     	}
	     	while( UART_RECEIVE_BUFFER[9]!=0x0 );
	 
	     	if(ucDelay_Exit!=1)
	     	{			
			printf("get another finger\n");
	 	 	Cmd_Img_To_Buffer2(); //将肯炕怀韶麈訠uffer2?
             		Recevice_Data(12);   //接?2缚靠?!?
	
	         	Cmd_Reg_Model();//炕怀韶?  
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
 User_auth()指纹验证
 Command ID:0x24
协议头	协议长度	固定字段	命令码	指纹序号	协议尾
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
        Cmd_Get_Img(); //发嘶窦厦?   
        Recevice_Data(12); //接?2缚靠?!?
	}
	while(UART_RECEIVE_BUFFER[9]!=0x0);
			   
        if(ucDelay_Exit!=0x1) //裙统? {
        Cmd_Img_To_Buffer1(); 
        Recevice_Data(12);   
			    
	Cmd_Search_Finger(); 
	Recevice_Data(16);   
         
	 
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //哦细嗡耸否?
	{
		printf("auth ok id =%d match score = %d\n",UART_RECEIVE_BUFFER[11],UART_RECEIVE_BUFFER[13]);
	}else{
		printf("auth error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 


}



/*****************************************************************
 User_erase()删除所指定指纹记录
 Command ID:0x22
协议头	协议长度	固定字段	命令码	指纹序号	协议尾
02	04		01 02		22	MM		02 01 03

******************************************************************/
void User_erase(unsigned char ucFPIdx){
	
}


/*****************************************************************
 User_clear()清除所有指纹(只有管理员才能进行操作,建议应用程序验证管理员身份)
 Command ID:0x26
协议头	协议长度	固定字段	命令码	无用字段	协议尾
02	04		01 02		26	任意值		02 01 03

******************************************************************/
void User_clear()
{
	Cmd_Delete_All_Model();
	Recevice_Data(12);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //哦细嗡耸否?
	{
		printf("clear ok \n");
	}else{
		printf("clear error \n");
	} 
	
}


/*****************************************************************
 Set_SecuLv()设置指纹比对时的严格等级(00-04)
 Command ID:0x28
协议头	协议长度	固定字段	命令码	安全级别	协议尾
02	04		01 02		28	MM		02 01 03
******************************************************************/
void Set_SecuLv(unsigned char ucSecuLv)
{
}


/*****************************************************************
 Get_FPNum()获得当前已注册指纹数
 Command ID:0x27
协议头	协议长度	固定字段	命令码	协议尾
02	03		01 01		27	02 01 03
******************************************************************/

void Get_FPNum()
{
	
	Cmd_Get_Model_Num(); 
	Recevice_Data(14);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //哦细嗡耸否?
	{
		printf("finger num  =%d \n",UART_RECEIVE_BUFFER[11]);
	}else{
		printf("get error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 

}


/*****************************************************************
 FPM_Start()启动指纹模块
 协议头	协议长度	固定字段	波特率（4字节）	固定字段
(28字节)	协议尾
02	3B	01 39 16 00 00 00 00 50 00 50 00 00 00 0000 00 00 00 64 00 00 00 00 	MM MM MM MM	CC CC CC71 00 00 10 01 00 00 00 00 00 00 00 00 00 01 0010 00 12 00 10 00 00 00 00 00 00 00 00	02 01 03
******************************************************************/
void FPM_Reset()
{
}

/*****************************************************************
 FPM_Reset()复位指纹模块
******************************************************************/
void FPM_Start()
{
}

