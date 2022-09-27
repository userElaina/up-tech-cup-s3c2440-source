#include"fingermap.h"
#include "finger_process.h"
/**
*ÓÉÓÚ¹«Ë¾Ìá¹©µÄÍ¨Ñ¶ÖÐ£¬·µ»ØÐÅÏ¢µÄ´óÐ¡²»Ò»£¬Òò´ËÎÞ·¨²ÉÓÃÍ³Ò»µÄ¶Ë¿ÚÀ´´¦ÀíËùÓÐµÄ²ÎÊýÐÅÏ¢£»
*
*ÕâÀï²ÉÓÃµÄ·½·¨ÊÇ½«£¬Ã¿´Î²É¼¯µ½µÄrxbuff´«µÝ¸Ã´¦Àíº¯Êý£¬
*
*×îºóÓÉ´¦Àíº¯ÊýÀ´ÅÐ¶ÏÐÅÏ¢µÄ´óÐ¡£¬²¢×ö³öÏàÓ¦µÄ´¦Àí
*
* @date 2007 08 21
* @auther lyj_uptech@126.com
*
*/

#define  DELAY_TIME  75

/*****************************************************************
 User_enroll()×¢²áÓÃ»§
 Command ID:0x21
Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	ÃüÁîÂë	Ö¸ÎÆÐòºÅ	Ð­ÒéÎ²
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
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Å¶Ï¸ÎËËÊ·ñ¦
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
     		Cmd_Get_Img(); //»ñ¸Î¿Ï
	 	Recevice_Data(12); //½ÓÕ2¸¿¿¿´!Â
     	}while ( UART_RECEIVE_BUFFER[9]!=0x00 ); //¼ìÊ·ñ¦µ¿´Á¿Î
     	if(ucDelay_Exit!=1){
		printf("get one finger\n");
		 
         	Cmd_Img_To_Buffer1(); //½«¿Ï¿»»³ÉØ÷æÔBuffer1Ö
	     	Recevice_Data(12);   //½ÓÕ2¸¿¿¿´!Â
		
		//check if this finger already in store
		Cmd_Search_Finger(); 
	        Recevice_Data(16);   
             	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Å¶Ï¸ÎËËÊ·ñ¦i
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
		      	Cmd_Get_Img(); //»ñ¸Î¿Ï
		      	Recevice_Data(12); //½ÓÕ2¸¿¿¿´!Â			 
	     	}
	     	while( UART_RECEIVE_BUFFER[9]!=0x0 );
	 
	     	if(ucDelay_Exit!=1)
	     	{			
			printf("get another finger\n");
	 	 	Cmd_Img_To_Buffer2(); //½«¿Ï¿»»³ÉØ÷æÔBuffer2Ö
             		Recevice_Data(12);   //½ÓÕ2¸¿¿¿´!Â
	
	         	Cmd_Reg_Model();//¿»»³ÉØ÷  
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
 User_auth()Ö¸ÎÆÑéÖ¤
 Command ID:0x24
Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	ÃüÁîÂë	Ö¸ÎÆÐòºÅ	Ð­ÒéÎ²
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
        Cmd_Get_Img(); //·¢Ë»ñ¼ÏÃÁ   
        Recevice_Data(12); //½ÓÕ2¸¿¿¿´!Â
	}
	while(UART_RECEIVE_BUFFER[9]!=0x0);
			   
        if(ucDelay_Exit!=0x1) //È¹û³¬¿Í³ö {
        Cmd_Img_To_Buffer1(); 
        Recevice_Data(12);   
			    
	Cmd_Search_Finger(); 
	Recevice_Data(16);   
         
	 
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Å¶Ï¸ÎËËÊ·ñ¦
	{
		printf("auth ok id =%d match score = %d\n",UART_RECEIVE_BUFFER[11],UART_RECEIVE_BUFFER[13]);
	}else{
		printf("auth error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 


}



/*****************************************************************
 User_erase()É¾³ýËùÖ¸¶¨Ö¸ÎÆ¼ÇÂ¼
 Command ID:0x22
Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	ÃüÁîÂë	Ö¸ÎÆÐòºÅ	Ð­ÒéÎ²
02	04		01 02		22	MM		02 01 03

******************************************************************/
void User_erase(unsigned char ucFPIdx){
	
}


/*****************************************************************
 User_clear()Çå³ýËùÓÐÖ¸ÎÆ(Ö»ÓÐ¹ÜÀíÔ±²ÅÄÜ½øÐÐ²Ù×÷,½¨ÒéÓ¦ÓÃ³ÌÐòÑéÖ¤¹ÜÀíÔ±Éí·Ý)
 Command ID:0x26
Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	ÃüÁîÂë	ÎÞÓÃ×Ö¶Î	Ð­ÒéÎ²
02	04		01 02		26	ÈÎÒâÖµ		02 01 03

******************************************************************/
void User_clear()
{
	Cmd_Delete_All_Model();
	Recevice_Data(12);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Å¶Ï¸ÎËËÊ·ñ¦
	{
		printf("clear ok \n");
	}else{
		printf("clear error \n");
	} 
	
}


/*****************************************************************
 Set_SecuLv()ÉèÖÃÖ¸ÎÆ±È¶ÔÊ±µÄÑÏ¸ñµÈ¼¶(00-04)
 Command ID:0x28
Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	ÃüÁîÂë	°²È«¼¶±ð	Ð­ÒéÎ²
02	04		01 02		28	MM		02 01 03
******************************************************************/
void Set_SecuLv(unsigned char ucSecuLv)
{
}


/*****************************************************************
 Get_FPNum()»ñµÃµ±Ç°ÒÑ×¢²áÖ¸ÎÆÊý
 Command ID:0x27
Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	ÃüÁîÂë	Ð­ÒéÎ²
02	03		01 01		27	02 01 03
******************************************************************/

void Get_FPNum()
{
	
	Cmd_Get_Model_Num(); 
	Recevice_Data(14);   	
	if( UART_RECEIVE_BUFFER[9]==0x0 )  //Å¶Ï¸ÎËËÊ·ñ¦
	{
		printf("finger num  =%d \n",UART_RECEIVE_BUFFER[11]);
	}else{
		printf("get error : %x\n",UART_RECEIVE_BUFFER[9]);
	} 

}


/*****************************************************************
 FPM_Start()Æô¶¯Ö¸ÎÆÄ£¿é
 Ð­ÒéÍ·	Ð­Òé³¤¶È	¹Ì¶¨×Ö¶Î	²¨ÌØÂÊ£¨4×Ö½Ú£©	¹Ì¶¨×Ö¶Î
(28×Ö½Ú)	Ð­ÒéÎ²
02	3B	01 39 16 00 00 00 00 50 00 50 00 00 00 0000 00 00 00 64 00 00 00 00 	MM MM MM MM	CC CC CC71 00 00 10 01 00 00 00 00 00 00 00 00 00 01 0010 00 12 00 10 00 00 00 00 00 00 00 00	02 01 03
******************************************************************/
void FPM_Reset()
{
}

/*****************************************************************
 FPM_Reset()¸´Î»Ö¸ÎÆÄ£¿é
******************************************************************/
void FPM_Start()
{
}

