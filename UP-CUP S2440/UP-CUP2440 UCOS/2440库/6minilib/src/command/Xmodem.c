/*----------------------------------------------------------------------------------
作者 孙恺
2003 6 11
Xmodem协议文件传送

说明
	通过串口按照Xmodem协议接受发送文件。
	带CRC校验，包128字节，接受有错误处理。

todo:	
	添加发送过程中的包报发送失败处理。
-----------------------------------------------------------------------------------*/

#include "../inc/command/xmodem.h"
#include "../inc/sys/lib.h"
#include <string.h>
#include <stdio.h>

#define Debug_Printf	printfNULL

int calcrc(unsigned char *ptr, int count)
{
    int crc, i;
	crc = 0;
    while(--count >= 0) {
	crc = crc ^ (int)*ptr++ << 8;
	for(i = 0; i < 8; ++i)
	    if(crc & 0x8000)
		crc = crc << 1 ^ 0x1021;
	    else
		crc = crc << 1;
	}
    return (crc & 0xFFFF);
}


int XReceiveFile(char * filename,int filetype)
{
	static unsigned char data[133];
	int i,j,crc,crcr;
	int error=FALSE,errornum=0;
	char paknum=1;
	int packagecon=1;

	FILE *file1;

	file1=fopen(filename, "w");
	if(file1==NULL) {
		return FAIL;
	}
	
	Debug_Printf("Wait to begin receice FILE.\n");
	for(i=0;;i++) {
		putchar(0x43);
		data[0]=0;
		getchartimeout(data, 1, 10000);
		if(data[0]==XMODEM_SOH) {
			Debug_Printf("Begin receice FILE.\n");
			break;
		}
		else {
			Debug_Printf("Try %d times.\n",i);
		}
		if(i==100) {
			Debug_Printf("Reach Max TRY Number!\n");
			fclose(file1);
			return FAIL;
		}
	}
	getchartimeout(data+1,132,1000);
	for(i=0;;i++) {
		switch(data[0]) {
			case XMODEM_SOH:
				crc=calcrc(data+3,128);
				crcr=data[131];
				crcr=crcr<<8;
				crcr=crcr|data[132];
				if(data[1]!=paknum) {
					if(data[1]==(paknum-1)) {
						Debug_Printf("'blk' error:%d!=%d(should),\n",data[1],paknum);
						Debug_Printf("Perhaps master not receive lastv XMODEM_ACK!!\n");
						Debug_Printf("Goon!!!\n");
						hudelay(2);
						error=FALSE;
						paknum--;
						putchar(XMODEM_ACK);
					}
					else {
						Debug_Printf("'blk' error:%d!=%d(should)\n",data[1],paknum);
						error=TRUE;
					}
				}
				else if(data[2]!=0xff-data[1]) 	{
					Debug_Printf("'255-blk' error\n");
					error=TRUE;		
				}
				else if(crc!=crcr) {
					Debug_Printf("crc error.:%x!=%x(should).\n",crcr,crc);
					error=TRUE;
				}
				else {
					Debug_Printf("Receive package:%d\n",packagecon);
					error=FALSE;
					if(filetype==XMODEM_DAT) {
						fwrite(data+3,128, 1, file1);
					}
					else if(filetype==XMODEM_TXT) {
						if(data[130]==0x1a) {
							for(j=0;j<128;j++) {
								if(data[j+3]==0x1a)
									break;
							}
							fwrite(data+3,j, 1, file1);
						}
						else
							fwrite(data+3, 128, 1, file1);
					}
					putchar(XMODEM_ACK);
					data[131]=0;
					paknum++;
					packagecon++;
				}
				break;
			case XMODEM_EOT:
				Debug_Printf("Receive finished.\n");
				putchar(XMODEM_ACK);
				fclose(file1);
				return TRUE;
				break;
			default:
				Debug_Printf("Unknowen Command!\n");
				error=TRUE;
				break;
		}
		if(error==TRUE) {
			errornum++;
			if(errornum==100) {
				Debug_Printf("Reach Max Receive ERROR Number!\n");
				fclose(file1);
				return FALSE;
			}
			else {
				Debug_Printf("Error:NO.%d,PackNum:%d,blk=%d,crc=%x\n",errornum,packagecon,data[1],crcr);
				mdelay(1);
				//OSFlushUart0();
				putchar(XMODEM_NAK);
			}
		}
		data[0]=0;
		data[1]=0;
		data[2]=0;
		getchartimeout(data,133,1000);
	}
	
}

int XTransmitFile(char * filename,int filetype)
{
	static unsigned char data[133];
	int i,j,crc;
	int errornum=0;
	unsigned int paknum=1,fullpacknum;
	FILE *file1;
	int b128;
	int finish;

	char fillchar=0;

	//get file size
	file_stat s;
	lstat(filename, &s);


	finish=FALSE;
	
	file1=fopen(filename, "r");
	if(file1==NULL) {
		return FALSE;
	}

	if(filetype==XMODEM_DAT)
		fillchar=0x1a;
	else if(filetype==XMODEM_TXT)
		fillchar=' ';
	fullpacknum=s.st_size/128;
	if((s.st_size%128)==0) {
		b128=TRUE;
	}
	else {
		b128=FALSE;
	}

	Debug_Printf("File Size %d.\n",s.st_size);
	Debug_Printf("Total %d Package.\n",fullpacknum);
 	hudelay(25);
	Debug_Printf("Wait to Begin transmit FILE.\n");
	for(i=0;;i++) {
		data[0]=0;
		getchartimeout(data,1,1000);
		if(data[0]==0x43) {
			Debug_Printf("Begin transmit FILE.\n");
			break;
		}
		else {
			Debug_Printf("\nWait %d times.",i);
		}
		if(i==600) {
			Debug_Printf("Reach Max Wait Number!\n");
			fclose(file1);
			return FALSE;
		}
	}
	data[0]=XMODEM_ACK;

	for(i=0;;i++) {
		if(data[0]==XMODEM_ACK) {
			if(paknum<=fullpacknum) {
				data[0]=XMODEM_SOH;
				data[1]=paknum;
				data[2]=0xff-paknum;
				fread(data+3,128, 1, file1);
				crc=calcrc(data+3,128);
				data[131]=crc>>8;
				data[132]=crc;
				for(j=0;j<133;j++)
					putchar(data[j]);
				paknum++;
				Debug_Printf("Transmit %d package.\n",paknum);
			}
			else if((paknum==fullpacknum+1)&&(b128==FALSE)) {
				data[0]=XMODEM_SOH;
				data[1]=paknum;
				data[2]=0xff-paknum;
				if((s.st_size%128)==0) {
					fread(data+3, 128, 1, file1);
				}
				else {
					fread(data+3,s.st_size%128, 1, file1);
					for(j=s.st_size%128;j<131;j++) {
						data[3+j]=fillchar;
					}
				}
				crc=calcrc(data+3,128);
				data[131]=crc>>8;
				data[132]=crc;
				for(j=0;j<133;j++)
					putchar(data[j]);
				Debug_Printf("Transmit %d package, last one, filled.\n",paknum);
				paknum++;
			}
			else {
				if (finish==FALSE) {
					data[0]=XMODEM_EOT;
					putchar(data[0]);
					finish=TRUE;
				}
				else {
					Debug_Printf("Transmit Finished.\n");
					Debug_Printf("File Size %d.\n",s.st_size);
					Debug_Printf("Total %d Package + %d Package.\n",fullpacknum,(b128==FALSE));
					//OSFlushUart0();
					fclose(file1);
					return TRUE;
				}
			}
		}
		else if(data[0]==XMODEM_CAN) {
			Debug_Printf("Transmit canceled.\n");
			//OSFlushUart0();
			fclose(file1);
			return FALSE;
		}
		else if(data[0]==XMODEM_NAK) {
			errornum++;
			if(errornum==100) {
				Debug_Printf("Reach Max Transmit ERROR Number!\n");
				//OSFlushUart0();
				fclose(file1);
				return FALSE;
			}
			else {
				Debug_Printf("'XMODEM_NAK' found!,NO.%d\n",errornum);
				if (finish==FALSE) {
					for(j=0;j<133;j++)
						putchar(data[j]);
				}
				else {
					data[0]=XMODEM_EOT;
					putchar(data[0]);
				}
			}
		}
		getchartimeout(data,1,1000);
	}

	/*
	for(i=0;;i++)
	{
		Debug_Printf("\nReceive Command:0x%x",data[0]);
		if(data[0]==XMODEM_ACK)
		{
			Debug_Printf("\nTransmit NO.%d package.",paknum-1);
			if(paknum<fullpacknum)
			{
				data[0]=XMODEM_SOH;
				data[1]=paknum;
				data[2]=0xff-paknum;
				//memcpy(data+3,&(file1->Buffer[(paknum-1)*128]),128);
				ReadOSFile(file1,(U8 *)(data+3),128);
				crc=calcrc(data+3,128);
				data[131]=crc>>8;
				data[132]=crc;

				////
				if(20==paknum)
					data[3]=~data[3];
				////
				
				for(j=0;j<133;j++)
					putchar(data[j]);
				paknum++;
				
			}
			else if(paknum==fullpacknum)
			{
				data[0]=XMODEM_SOH;
				data[1]=paknum;
				data[2]=0xff-paknum;
				//memcpy(data+3,&(file1->Buffer[(paknum-1)*128]),file1->filesize%128);
				
				if((file1->filesize%128)==0)
				{
					ReadOSFile(file1,(U8 *)(data+3),128);
				}
				else
				{
					ReadOSFile(file1,(U8 *)(data+3),file1->filesize%128);
					for(j=file1->filesize%128;j<131;j++)
					{
						if(filetype==XMODEM_DAT)
							data[3+j]=0x1a;
						else if(filetype==XMODEM_TXT)
							data[3+j]=' ';
					}
				}
				crc=calcrc(data+3,128);
				data[131]=crc>>8;
				data[132]=crc;
				for(j=0;j<133;j++)
					putchar(data[j]);
				Debug_Printf("\nTransmit %d package.",paknum);
				paknum++;
			}
			else
			{
				//if (data[0]!=XMODEM_EOT)
				{
					data[0]=XMODEM_EOT;
					putchar(data[0]);
				}
				//else
				{
					Debug_Printf("\nTransmit Finished.");
					CloseOSFile(file1);
					OSReadUart0(data,1,1000);
					return TRUE;
				}
			}
		}
		else if(data[0]==XMODEM_NAK)
		{

			errornum++;
			if(errornum==10)
			{
				Debug_Printf("\nReach Max Transmit ERROR Number!");
				CloseOSFile(file1);
				return FALSE;
			}
			else
			{
				Debug_Printf("\n'XMODEM_NAK' found!,NO.%d",errornum);
				Debug_Printf("\nReTransmit:Package=%d",data[1]);
				udelay(1000);
				OSFlushUart0();
				for(j=0;j<133;j++)
					putchar(data[j]);
			}
		
		}
		else if(data[0]==XMODEM_CAN)
		{
			Debug_Printf("\nTransmit Canceled by Receiver!");
			CloseOSFile(file1);
			return FALSE;
		}
		else
		{	
			errornum++;
			if(errornum==10)
			{
				Debug_Printf("\nReach Max Transmit ERROR Number!");
				CloseOSFile(file1);
				return FALSE;
			}
			Debug_Printf("\nUnknowen Command!");
			udelay(30000);
		
		}
		data[0]=0;
		OSReadUart0(data,1,1000);
	}
	*/

}

int XReceive2Mem(unsigned char *pbuffer, unsigned int* n)
{
	static unsigned char data[133];
	int i,crc,crcr;
	int error=FALSE,errornum=0;
	char paknum=1;
	int packagecon=1;

	Debug_Printf("Wait to begin receice FILE.\n");
	for(i=0;;i++) {
		putchar(0x43);
		data[0]=0;
		getchartimeout(data, 1, 1000);
		if(data[0]==XMODEM_SOH) {
			Debug_Printf("Begin receice FILE.\n");
			break;
		}
		else {
			Debug_Printf("Try %d times.\n",i);
		}
		if(i==60) {
			Debug_Printf("Reach Max TRY Number!\n");
			return FAIL;
		}
	}
	getchartimeout(data+1,132,1000);
	for(i=0;;i++) {
		switch(data[0]) {
			case XMODEM_SOH:
				crc=calcrc(data+3,128);
				crcr=data[131];
				crcr=crcr<<8;
				crcr=crcr|data[132];
				if(data[1]!=paknum) {
					if(data[1]==(paknum-1)) {
						Debug_Printf("'blk' error:%d!=%d(should),\n",data[1],paknum);
						Debug_Printf("Perhaps master not receive lastv XMODEM_ACK!!\n");
						Debug_Printf("Goon!!!\n");
						hudelay(2);
						error=FALSE;
						paknum--;
						putchar(XMODEM_ACK);
					}
					else {
						Debug_Printf("'blk' error:%d!=%d(should)\n",data[1],paknum);
						error=TRUE;
					}
				}
				else if(data[2]!=0xff-data[1]) 	{
					Debug_Printf("'255-blk' error\n");
					error=TRUE;		
				}
				else if(crc!=crcr) {
					Debug_Printf("crc error.:%x!=%x(should).\n",crcr,crc);
					error=TRUE;
				}
				else {
					Debug_Printf("Receive package:%d\n",packagecon);
					error=FALSE;

					memcpy(pbuffer,(data+3),128);
					*n+=128;
					pbuffer+=128;

					putchar(XMODEM_ACK);
					data[131]=0;
					paknum++;
					packagecon++;
				}
				break;
			case XMODEM_EOT:
				Debug_Printf("Receive finished.\n");
				putchar(XMODEM_ACK);
				return TRUE;
				break;
			default:
				Debug_Printf("Unknowen Command!\n");
				error=TRUE;
				break;
		}
		if(error==TRUE) {
			errornum++;
			if(errornum==100) {
				Debug_Printf("Reach Max Receive ERROR Number!\n");
				return FALSE;
			}
			else {
				Debug_Printf("Error:NO.%d,PackNum:%d,blk=%d,crc=%x\n",errornum,packagecon,data[1],crcr);
				mdelay(1);
				//OSFlushUart0();
				putchar(XMODEM_NAK);
			}
		}
		data[0]=0;
		data[1]=0;
		data[2]=0;
		getchartimeout(data,133,1000);
	}
	
}

