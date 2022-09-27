#include "zigbee.h"

//aa ff 08 00 03 02 00 0C A0 01 ff
void zigbee_rev_data(uchar *addr,uchar n){
	uchar txbuff[11];	
	uchar rxbuff[20];	
	txbuff[0]=0xAA;
	txbuff[1]=0xFF;
	txbuff[2]=0x08;
	txbuff[3]=0x01;//rev cmd
	txbuff[4]=0x00;
	txbuff[5]=0x00;
	txbuff[6]=0x00;
	txbuff[7]=0x00;
	txbuff[8]=0x00;
	txbuff[9]=0x00;
	txbuff[10]=0xFF;
        tty_fflush();
	tty_write(txbuff,11);
        fflush(stdout);
//---------------------------------------------------
//BB FF 07 00 01 00 00 00 00 FF
//-----------------------------------------------------
//    printf("\n reading");
        fflush(stdout);
        tty_read(rxbuff,8);
//      printf("\nget it read");
        fflush(stdout);
        if(rxbuff[1]==0xFF)
          if(rxbuff[4]==0x01)
          {
                printf("success! \n");
          }else
		printf("failed! \n");
}
//aa ff 08 00 03 02 00 0C A1 01 ff
void zigbee_set_LED(uchar *addr,uchar cmd,uchar arg){
	uchar txbuff[11];	
	uchar rxbuff[11];	
	txbuff[0]=0xAA;
	txbuff[1]=0xFF;
	txbuff[2]=0x08;
	txbuff[3]=0x00;
	txbuff[4]=0x03;
	txbuff[5]=0x02;
	txbuff[6]=addr[0];
	txbuff[7]=addr[1];
	txbuff[8]=cmd;
	txbuff[9]=arg;
	txbuff[10]=0xFF;
	tty_write(txbuff,11);
        fflush(stdout);
#if 0
//---------------------------------------------------
//BB FF 07 00 01 00 00 00 00 FF
//-----------------------------------------------------
//    printf("\n reading");
        fflush(stdout);
        tty_read(rxbuff,10);
//      printf("\nget it read");
        fflush(stdout);
        if(rxbuff[1]==rxbuff[9])
          if(rxbuff[4]==0x01)
          {
                printf("success! \n");
          }else
                printf("failed! \n");
#endif
}
//…Ë÷√◊√//aa ff 08 00 03 02 00 0C A0 01 ff
void zigbee_set_default(){
        uchar txbuff[11];
        uchar rxbuff[20];
        txbuff[0]=0xAA;
        txbuff[1]=0xFF;
        txbuff[2]=0x07;
        txbuff[3]=0x10;//rev cmd
        txbuff[4]=0x00;
        txbuff[5]=0x0A;
        txbuff[6]=0x00;
        txbuff[7]=0x00;
        txbuff[8]=0x00;
        txbuff[9]=0x00;
        txbuff[10]=0xFF;
        tty_fflush();
        tty_write(txbuff,11);
        fflush(stdout);
//---------------------------------------------------
//BB FF 07 00 01 00 00 00 00 FF
//-----------------------------------------------------
//    printf("\n reading");
        fflush(stdout);
        tty_read(rxbuff,8);
//      printf("\nget it read");
        fflush(stdout);
        if(rxbuff[1]==0xFF)
          if(rxbuff[4]==0x01)
          {
                printf("success! \n");
          }else
                printf("failed! \n");
}
