#include "sensor.h"

//
void sensor(uchar cmd,int param){
	uchar txbuff[5];
	uchar rxbuff[7];
	int i = 0;
	txbuff[0]=0xAA;
	txbuff[1]=0xFF;
	txbuff[2]=cmd;
	txbuff[3]=0xFF;
	for(i=2;i<3;i++){
                txbuff[3]^=txbuff[i];
        }
	txbuff[4]=0xff;
	tty_write(txbuff,5);
        fflush(stdout);
        tty_read(rxbuff,7);
        fflush(stdout);
	printf("we get data:");
        for(i=2;i<5;i++){
              printf(" %x",rxbuff[i]);
        }
        rxbuff[6]=0;
        for(i=2;i<4;i++){
                rxbuff[5]^=rxbuff[i];
        }
//        if(rxbuff[6]==rxbuff[5]){
//		printf("\nCRC ok !\n");
//	}
}
