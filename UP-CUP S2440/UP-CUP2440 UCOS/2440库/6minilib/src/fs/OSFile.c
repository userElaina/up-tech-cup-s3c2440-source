/*----------------------------------------------------------------------------------
    #说明: 王晓君写的文件系统，(Fat12格式)
    #接口函数
	----------------------------------  Bug  --------------------------------------
	2003-6-12	全局变量Pre_Block、Current_Block未作处理，应该替换成
				局部变量，防止多任务冲突；

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-1-8	修改了文件分区表的格式，格式化的时候需要新的bootloader配合

	2003-6-13	OpenOSFile函数中引入Create模式，对于文件的Write操作，如
				果没有Create标志则只能替换已有的同名文件，如果有
				Create标志，则只能创建新的文件，如果有同名文件，
				在返回NULL

	2003-6-13	DeleteOSFile函数通过测试

	2003-6-13	对于Root表缓冲处理，减少Flash的磨损，只有在Write模式
				写入文件、创建root入口以及删除文件的时候写入Flash，
				其他的操作都通过内存。

			###可能的问题:文件系统中Fat和root表都做缓冲处理，在写入
				Flash的一瞬间系统掉电，就会造成不可预测的结果，
				可能导致整个文件系统的破坏。

	2003-6-13	对于文件名不区分大小写，存入的时候按照用户
				给定的名字保存，查找和比较的时候，不区分大小写
				
	2003-6-13	支持8.3格式的文件，同时兼容以前的11字符模式的 
				文件名输入

	2003-6-12	列出文件

	2003-6-12	可以写入大于一个Cluster的文件;DeleteOSFile函数未测试;
				把FTA分配表在内存中专门建立一个缓冲区，在
				CloseOSFile、DeleteOSFIle的时候对Flash进行同步
				
																			by threewater
	2003-6-11	修正CloseOSFile的时候更新分区表，写入文件长度 by threewater
	------------------------------------------------------------------------------
	
-----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------
//王晓君进行第二次修订所做修改,修改后版本号2.0
	2003-9-16
*/

#include "../ucos-ii/includes.h"               /* uC/OS interface */
#include "../inc/sys/lib.h"
#include "../inc/drivers.h"
#include "OSFile.h"
#include <stdio.h>
#include <string.h>


#undef DPRINTK

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DPRINTK			LCD_printf
#else
#define DPRINTK			printfNULL
#endif

#if (USE_YAFFS==0)

#define Erase_Cluster(n)		Erase_Block(n)

//文件分配表的大小(byte)
#define FAT_TABLE_SIZE	(TotalSector*3/2/SecPerClus-3)

static unsigned int TotalCluster,BootSector,RsdSector,SectorofFatSize,TotalCapacity,RootEntry,SecPerClus,TotalSector,BytesPerSec,FirstDataSec;
static unsigned char FAT_TYPE;

__align(4) static unsigned char databuff[0x200];
//unsigned char Buffer[Block_Size];


//U32 Pre_Block,Current_Block;


//unsigned char FileSystemFAT[16*1024]={0,};
__align(4) static unsigned char FileSystemFAT[1024]={0,};


typedef struct _FileRoot{
	char filename[11];	//0-10
	char Attribute;		//11		Attribute
	char pad[10];			//12-21
	U16 time;			//22,23	MSB??
	U16 date;			//24,25	MSB??
	U16 cluster;			//26,27	first cluster	LSB
	U32 filelength;		//28-31	file length 		LSB
}FileRoot,*PFileRoot;

__align(4) static FileRoot FileSystemRoot[FILESYS_MAX_BUFFER_FILEROOT]={0};

static OS_MEM *pFileMem;
__align(4) static INT8U FileMemPart[10][sizeof(OSFILE)];

static U8 BPB_Data [512]={
				0xeb,0x3c,0x90,//	Offset_U8_BS_jmpBoot		0
				'U','P','-','T','E','C','H',' ',// Offset_U8_BS_OEMName		3
				LOWBYTE(BytesPerSector),HIGHBYTE(BytesPerSector),	// Offset_U16_BPB_BytsPerSec	11	//Count of bytes per sector. This value may take on only the  //
				//512									//following values: 512, 1024, 2048 or 4096. If maximum       //
														//compatibility is desired, only the value 512 should be used.//
				64,// Offset_U8_BPB_SecPerClus	13	//Number of sectors per allocation unit. This value must be a //
														//power of 2 that is greater than 0. The legal values are 1,  //
														//	2, 4, 8, 16, 32, 64,and 128.                                //
				LOWBYTE(RSD_Sector),HIGHBYTE(RSD_Sector),// Offset_U16_BPB_RsvdSecCnt	14	//Number of reserved sectors in the Reserved region of the    //
														//volume starting at the first sector of the volume.For FAT32 //
														//volumes, this value is typically 32.                        //
				0x02,// Offset_U8_BPB_NumFATs		16	//The count of FAT data structures on the volume.             //
				LOWBYTE(Directory_Number),HIGHBYTE(Directory_Number),// Offset_U16_BPB_RootEntCnt	17	//Is only useful for FAT12 and FAT16 volumes.                 //
				//										//For FAT32 volumes,this field must be set to 0.              //
				//通过调整文件项为03b0，从而使数据扇区刚好从Block2Sector0开始
				LOWBYTE(Total_Sector),HIGHBYTE(Total_Sector),// Offset_U16_BPB_TotSec16		19	//For FAT12 and FAT16 volumes, this field contains the sector //
														//count, and BPB_TotSec32 is 0 if the total sector count fits //
														//(is less than 0x10000).                                     //
				0xf8,// Offset_U8_BPB_Media			21	//0xF8 is the standard value for fixed(non-removable) media.  //
														//For removable media, 0xF0 is frequently used.               //
				LOWBYTE(FAT_NUM),HIGHBYTE(FAT_NUM),// Offset_U16_BPB_FATSz16		22	//number of sectors in FAT

				0x00,0x00,// Offset_U16_BPB_SecPerTrk	24	//Sectors per track for interrupt 0x13. This field is only    //
														//relevant for media that have a geometry (volume is broken   //
														//down into tracks by multiple heads and cylinders) and are   //
														//visible on interrupt 0x13.This field contains the           //
														//"sectors per track" geometry value.                         //
				0x00,0x00,// Offset_U16_BPB_NumHeads		26	//Number of heads for interrupt 0x13. This field is relevant  //
														//as discussed earlier for BPB_SecPerTrk.                     //
				0x01,0x00,0x00,0x00,// Offset_U32_BPB_HiddSec		28	//Count of hidden sectors preceding the partition that        //
														//contains this FAT volume.                                   //
				0x00,0x00,0x00,0x00,// Offset_U32_BPB_TotSec32		32	//This field is the new 32-bit total count of sectors on the  //
														//volume. This count includes the count of all sectors in all //
														//four regions of the volume. This field can be 0; if it is 0,//
														//then BPB_TotSec16 must be non-zero. For FAT32 volumes, this //
														//field must be non-zero. For FAT12/FAT16 volumes, this field //
														//contains the sector count if BPB_TotSec16 is 0 (count is    //
														//greater than or equal to 0x10000).//
				0x00,// Offset_U8_BS_DrvNum		36	//This field is the FAT32 32-bit count of sectors occupied by //
					//根据该值得到FAT的大小//				//ONE FAT. BPB_FATSz16 must be 0.                             //
					
				0x00,// Offset_U8_BS_Reserved		37	//    Bits 0-3 -- Zero-based number of active FAT.Only valid  //
														//                if mirroring is disabled.                   //
														//    Bits 4-6 -- Reserved.                                   //
														//    Bit 7    -- 0 means the FAT is mirrored at runtime into //
														//                  all FATs.                                 //
														//             -- 1 means only one FAT is active;it is the one//
														//                  referenced in bits 0-3.                   //
														//    Bits 8-15 -- Reserved.                                  //
				0x29,// Offset_U8_BS_BootSig		38	//High byte is major revision number.                         //
														//Low byte is minor revision number.                          //
				'N','A','M','E',// Offset_U32_BS_VolID		39	//This is set to the cluster number of the first cluster of   //
					//根据该值得到ROOT的地址//				//the root directory, usually 2 but not required to be 2.     //
					
				'N','O',' ','N','A','M','E',' ',' ',' ',' ',// Offset_U2048_BS_VolLab		43	//Sector number of FSINFO structure in the reserved area of   //
														//the FAT32 volume.Usually 1.                                 //
				'F','A','T','1','2',' ',' ',' ',// Offset_U256_BS_FilSysType	54	//If non-zero, indicates the sector number in the reserved    //
														//area of the volume of a copy of the boot record.            //
				//Executable Code
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	

				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,	
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,

				0x00,//active partion
				0x00,//head
				0x00,//partion begin
				0x00,//cylinder
				0x06,//is partion used
				0x00,//end head
				0x00,//partion end 
				0x00,//end cylinder
				0x00,0x00,0x00,0x00,
				0x00,0x80,0x00,0x00,//fist partion

				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,//second partion
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,//third partion
				0,0,0,0,0,		0,0,0,0,0,		0,0,0,0,0,		0,//forth partion

				0x55,0xAA	// Offset_U16_Signature		510	//0x55AA           

				};
//////////////////////////////////////////////////////////////////////////////////////////
int initOSFile(void);
OSFILE* OpenOSFile(char filename[], U32 OpenMode);
U32 ReadOSFile(OSFILE* pfile,U8* ReadBuffer, U32 nReadbyte);
U32 LineReadOSFile(OSFILE* pfile, char str[]);	//读取指定文件的一行
U32 SeekOSFile(OSFILE* pfile ,U32 nCurPos);
U32 GetPosOSFile(OSFILE* pfile);
U8 DeleteOSFile(char filename[]);
U8 RenameOSFile(char fromname[], char toname[]);
U8 CopyOSFile(char srcfile[], char decfile[]);
int FindOSFile(char filename [ ]);
U8 WriteOSFile(OSFILE* pfile,U8* WriteBuffer, U32 nWriteyte);
int CloseOSFile(OSFILE* pfile);
//得到指定位置的文件名（包括扩展名），文件位置自动下移
U8 GetNextFileName(U32 *filepos,char filename[]);
//列出当前位置开始第一个指定扩展名的文件，如果没有，则返回FALSE
U8 ListNextFileName(U32 *filepos, char FileExName[],char filename[]);

void  Write2Flash(unsigned int block,unsigned int StSector,unsigned int EnSector,unsigned char* ClusterBuf);

int Init_FAT_Info(int AutoFormat);

unsigned int NextCluster(unsigned int CurrentCluster);
unsigned int AllocateCluster(unsigned int CurrentCluster);

void ReadFAT2Mem(void);
void ReadFileRoot2Mem(void);

/********************************************************************/
/*如果Flash的MBR和Fat16结构受损,则调用此函数可以恢*/	
/*复																*/
/********************************************************************/
int Format_Fat12Media(void);
void WriteMBR2Flash(void);
void CreatFAT16(void);
void CreatDirectoryEntry(void);


/********************************************************************/
/*测试文件系统功能时,使用以下几个函数进行		*/	
/********************************************************************/

void TestFAT_COPY(char filename1[],char filename2[]);
void TestFAT_CREATE(char filename[]);
void TestFAT_DELETE(char filename[]);
void TestFAT_READ(char filename[]);


/*################################################################*/
//以下代码完成介质的格式化,当介质出现读取故障时,可调用
//Format_Fat12Media函数实现对16M介质的格式化

//数据结构与全局变量定义

//写入BPB
void WriteMBR2Flash(void)
{
	WritePage(Begin_Cluster, BPB_Sector, BPB_Data);
}
//创建FAT16
void CreatFAT12(void)
{
	U8 FAT[512]={0,};

	memset(FAT,0,sizeof(FAT));
	
	//disk type lower byte is equal to Offset_U8_BPB_Media
	FAT[0] = 0xf8;
	FAT[1] = 0xff;//system occupied the first cluster
	FAT[2] = 0xff;
	FAT[3] = 0x7f;
	
	if(NandFlashOK()){
		WritePage(Begin_Cluster,Fat_Sector,FAT);	//Sector 1 of Fat16
		WritePage(Begin_Cluster,Fat_Sector+2,FAT);	//Sector 1 of Fat16
		FAT[0]=FAT[1]=FAT[2]=FAT[3]=0x00;
		WritePage(Begin_Cluster,Fat_Sector+1,FAT);	//Sector 2 of Fat16
		WritePage(Begin_Cluster,Fat_Sector+3,FAT);	//Sector 2 of Fat16
		}
	else
		DPRINTK("No Flash Found\n");

	ReadFAT2Mem();
}

//创建目录项
void CreatDirectoryEntry(void)
{
	static U8 Directory_Enties[512]={0,};
	U32 i;

	for(i=0;i<0x3b;i++) {//总共支持3b0个根目录项，共占用0x3b个扇区
		WritePage(Begin_Cluster+(i+Directory_BeginSector)/32,(Directory_BeginSector+i)%32,Directory_Enties);
	}	

	ReadFileRoot2Mem();
}


/***************************************************************************\
*	介质的格式化,当介质出现读取故障时,可调用			*
*	Format_Fat12Media函数实现对16M介质的格式化					*
\***************************************************************************/
int Format_Fat12Media(void)
{	

	printk("\nFormat the Media to FAT,Please Waiting...\n");

	Erase_Cluster(Begin_Cluster);
	Erase_Cluster(Begin_Cluster+1);
	
	WriteMBR2Flash();
	Init_FAT_Info(FALSE);
	
	CreatFAT12();
	CreatDirectoryEntry();
	
	printk ("Format Finished!\n");

	return TRUE;
}

/****************************************
	按照扇区(Page)写入Flash，
	参数:
			block 表示Nand Flash中Block的位置
			StSector 表示Nand Flash中写入的起始扇区
			EnSector 表示Nand Flash中写入的结束扇区(写入数据包含该扇区)
			ClusterBuf 表示写入数据，以Block的大小为单位对齐
			
*****************************************/

void  Write2Flash(unsigned int block,unsigned int StSector,unsigned int EnSector,unsigned char* ClusterBuf)
{
	unsigned int i;
	for(i=0;i<StSector;i++){
		ReadPage(block,i,ClusterBuf+i*NandPageSize);	//前前面的扇区读出
	}
	for(i=EnSector+1;i<32;i++){
		ReadPage(block,i,ClusterBuf+i*NandPageSize);	//前前面的扇区读出
	}
	Erase_Cluster(block);
	for(i=0;i<32;i++){
		WritePage(block,i,ClusterBuf+i*NandPageSize);	//前前面的扇区读出
	}
}


//add by threewater ------>
/****************************************
	读取FAT分区表到内存的缓冲区中
*****************************************/
void ReadFAT2Mem()
{
	int i;

	for(i=0;i<SectorofFatSize;i++)
//	for(i=0;i<2;i++)//16M的介质只有两个扇区1024字节的分区表空间-->by frank
		ReadPage(Begin_Cluster+(BootSector+RsdSector+i)/SecPerClus,
							(BootSector+RsdSector+i)%SecPerClus,FileSystemFAT+i*512);
	
}

/****************************************
	写入FAT分区表到内存的缓冲区中
*****************************************/
static void WriteFATFromMem()
{
	INT8U err;
	OSFILE *pfile;
	unsigned int StSector=(BootSector+RsdSector)%SecPerClus;//StSector并未得到引用

	pfile=(OSFILE*)OSMemGet(pFileMem,&err);//由于需要缓冲区，故创建一个指针

	memcpy(pfile->Buffer+StSector*512,FileSystemFAT,512*SectorofFatSize);
	Write2Flash(Begin_Cluster+(BootSector+RsdSector)/SecPerClus, 
				StSector, StSector+SectorofFatSize-1, pfile->Buffer);

	OSMemPut(pFileMem, pfile);

}

/****************************************
	从FAT分区表缓冲区中删除一个文件的链表
	参数Cluster表示文件的起始位置(Cluster)
*****************************************/
void DeleteFATList(unsigned int Cluster)
{
	unsigned int NxtCluster;
	switch(FAT_TYPE){
	case FAT12:
		while(((Cluster&0xfff)!=0xfff) && (Cluster!=0)){
			NxtCluster=NextCluster(Cluster);
			if(Cluster%2==0) {
				FileSystemFAT[Cluster*3/2]=0;
				FileSystemFAT[(Cluster*3/2)+1]&=0xf0;	//清空FAT中高8位的低半字节
			}
			else {
				FileSystemFAT[(Cluster*3/2)]&=0xf;	//清空FAT中低8位的高半字节
				FileSystemFAT[Cluster*3/2+1]=0;
			}
			Cluster=NxtCluster;
		}
		break;
	}
}

/****************************************
	读取根目录表到缓冲区中
*****************************************/
void ReadFileRoot2Mem()
{
	int j;
	//从根目录表的第一个扇区开始读取
	int i=BootSector+RsdSector+2*SectorofFatSize;
	unsigned char *p=(unsigned char*)FileSystemRoot;

	for(j=0;j<RootEntry*32/BytesPerSec;j++,i++){
		ReadPage(Begin_Cluster+i/PagePerClus,i%PagePerClus, p);
		p+=512;
	}

}

/****************************************
	从根目录表到缓冲区写入到Flash中
*****************************************/
void WriteFileRootFromMem()
{
	INT8U err;
	unsigned char *pbuffer;
	unsigned int StSector, nSector;
	unsigned int block, lastrootSector;
	unsigned char *p=(unsigned char*)FileSystemRoot;
	

	//TODO: 
	//根目录表的第一个扇区
	pbuffer=(unsigned char*)OSMemGet(pFileMem,&err);//由于需要缓冲区，故创建一个指针

	StSector=BootSector+RsdSector+2*SectorofFatSize;
	block=StSector/PagePerClus;

	lastrootSector=RootEntry*32/BytesPerSec;

	for(; lastrootSector>0; lastrootSector-=nSector){
		//一个Page中，从i的起始到page结束，或者剩余的rootsector个数的Sector数目
		nSector=min(PagePerClus-(StSector%PagePerClus), lastrootSector);

		memcpy(pbuffer+StSector*512,p,512*nSector);
		Write2Flash(Begin_Cluster+block, StSector, StSector+nSector, pbuffer);
		block++;
		p+=512*PagePerClus;
		StSector=0;	//以后的起始扇区始终为0
	}

	OSMemPut(pFileMem, pbuffer);
	
}

//以上代码完成文件分配表和根目录表在介质和内存之间的缓存
/*################################################################*/




/****************************************
	转换8.3格式的文件名为
	含有空格的11个字符的文件名
	如果没有.字符(非8.3格式)，则直接返回
*****************************************/
void FormatFileName(char outfilename[], char infilename[])
{
	int i;
	char *pch=infilename;

	for(i=0;i<11 && *pch && *pch!='.'; i++){
		outfilename[i]=*pch;
		pch++;
	}
	
	if(i==11 && *pch!='.'){
		memcpy(outfilename,infilename,11);
		return;
	}

	//找到. 位置在i
	if(i<8)
		memset(outfilename+i,' ',8-i);

	pch++;

	for(i=0;i<3 && *pch ;i++){
		*(outfilename+8+i)=*pch;
		pch++;
	}

	if(i<3)
		memset(outfilename+8+i,' ',3-i);
}

char mytoupper(char c)
{
	if(c>='a' && c<='z')//小写字母则转换成大写
		return c-0x20;

	return c;
}

/****************************************
	不区分大小写字母的比较
	如果相同则返回0，如果m1>m2则返回值>0，如果m1<m2则返回值<0
*****************************************/
int MemcmpNoUpper(char* m1, char *m2, int n)
{
	for(;mytoupper(*m1)==mytoupper(*m2);n--){
		m1++;
		m2++;
		if(n==1)		return 0;
	}
	return *m1-*m2;
}

//<-----------by threewater



/****************************************
	按照扇区(Page)写入Flash，
	参数:
			block 表示Nand Flash中Block的位置
			StSector 表示Nand Flash中写入的起始扇区
			EnSector 表示Nand Flash中写入的结束扇区(写入数据包含该扇区)
			ClusterBuf 表示写入数据，以Block的大小为单位对齐
			
*****************************************/


//add by threewater ------>
/****************************************
	擦除文件系统的簇，根据分区表信息自动计算文件系统的
	簇的大小，并擦除。
*****************************************/
void Erase_FileCluster(unsigned int filecluster)
{
	//计算文件系统中的一个Cluster相当于几个实际的Nand Flash的Block，
	//Nand Flash的Block大小为16KB
	int n=SecPerClus*BytesPerSec/(PagePerClus*BytesPerPage);
	int i;

	for(i=0;i<n;i++)
		Erase_Cluster(Begin_Cluster+(filecluster-1)*n+i);
}

/****************************************
	向缓冲区中的FAT表写入数据
*****************************************/
void Write2MemFat(unsigned int Cluster, unsigned int data)
{
	switch(FAT_TYPE){
	case FAT12:
		if(Cluster%2==0) {
			FileSystemFAT[Cluster*3/2]=data;
			data>>=8;
			data&=0xf;
			FileSystemFAT[(Cluster*3/2)+1]&=0xf0;
			FileSystemFAT[(Cluster*3/2)+1]|=data;	//写入FAT中高8位的低半字节
		}
		else {
			FileSystemFAT[(Cluster*3/2)]&=0x0f;
			FileSystemFAT[(Cluster*3/2)] |= ((data<<4)&0xf0);	//写入FAT中低8位的高半字节
			FileSystemFAT[Cluster*3/2+1] = (data>>4);
		}
		break;
	}
}
//<------add by threewater 



int initOSFile()
{
	INT8U err;

	pFileMem=OSMemCreate(FileMemPart,10, sizeof(OSFILE), &err);
	if(pFileMem==NULL){
		DPRINTK("Failed to Create OSFILE quote\n");
		DPRINTK("Failed to Create OSFILE quote\n");
	}
	return Init_FAT_Info(FILESYSTEM_AUTOFORMAT);
}


OSFILE* OpenOSFile(char filename[], U32 OpenMode)
{

	int i=0,j=0;
	U32 Pre_Cluster;
	unsigned int CurrentSector,nFileLength,SecNumOfClus;
//	short int EmptyDIRSector,EmptyDIREntry;
	OSFILE* pfile;
	INT8U err;
	char filename1[12]={0};
	int emptyrootpos=-1;	//空根目录处

	FormatFileName(filename1,filename);

	for(j=0; j<RootEntry;j++){	//每个扇区有多个目录项
		if( (FileSystemRoot[j].filename[0]==0 || FileSystemRoot[j].filename[0]==OSFILE_DELETEMARK)
			&& emptyrootpos==-1)
			emptyrootpos=j;	//记录空根目录处

		if(!MemcmpNoUpper(filename1, FileSystemRoot[j].filename,11)){	//如果找到某项符合条件
			DPRINTK("\nFile %s found!",filename1);
			pfile=(OSFILE*)OSMemGet(pFileMem,&err);
			pfile->fileCluster=FileSystemRoot[j].cluster;
			pfile->filesize=nFileLength=FileSystemRoot[j].filelength;
			pfile->filebufnum=0;
			pfile->fileCurpos=0;
			pfile->filemode=OpenMode;
			pfile->rootpos=j;

			switch(OpenMode){
			case FILEMODE_READ:
				while((pfile->fileCluster!=0xffff)&&((pfile->fileCluster&0xfff)!=0xfff)&&(pfile->fileCluster!=0xffffffff)){
					CurrentSector=(pfile->fileCluster-2)*SecPerClus+FirstDataSec;
					for(SecNumOfClus=0;
									(SecNumOfClus<SecPerClus)&&(pfile->filesize>pfile->filebufnum);
									SecNumOfClus++)
						{//读完文件,如果大于一簇,则先读一簇
						ReadPage(Begin_Cluster+(CurrentSector+SecNumOfClus)/PagePerClus,(CurrentSector+SecNumOfClus)%PagePerClus,databuff);
						if(nFileLength>512) memcpy(pfile->Buffer+pfile->filebufnum, databuff,512);
						else memcpy(pfile->Buffer+pfile->filebufnum, databuff,nFileLength);
						nFileLength-=512;
						pfile->filebufnum+=512;
						}
									
					if(pfile->filebufnum>=Block_Size) 
						{//读到1个Block后即中止
						pfile->filebufnum=0;//缓冲区指针重新指向开头			
						break;//缓冲区慢时中止
						}
 					else pfile->fileCluster=NextCluster(pfile->fileCluster);//如果缓冲区没满则继续读
				}
				printk("\nFile %s have been read!",filename1);
				pfile->filebufnum=0;//缓冲区指针重新指向开头			
				break;
			case FILEMODE_WRITE:
				if(pfile->filesize>0){//读出原有内容by frank Sep 21
					while((pfile->fileCluster!=0xffff)&&(pfile->fileCluster!=0xfff)&&(pfile->fileCluster!=0xffffffff)){
						CurrentSector=(pfile->fileCluster-2)*SecPerClus+FirstDataSec;
						for(SecNumOfClus=0;(SecNumOfClus<SecPerClus)&&(pfile->filesize>pfile->filebufnum);SecNumOfClus++){//读完文件,如果大于一簇,则先读一簇
							ReadPage(Begin_Cluster+(CurrentSector+SecNumOfClus)/PagePerClus,(CurrentSector+SecNumOfClus)%PagePerClus,databuff);
							if(nFileLength>512) memcpy(pfile->Buffer+pfile->filebufnum, databuff,512);
							else memcpy(pfile->Buffer+pfile->filebufnum, databuff,nFileLength);
							nFileLength-=512;
							pfile->filebufnum+=512;
							}
						if(pfile->filebufnum>=Block_Size) {//读到1个Block后即中止
							pfile->filebufnum=0;//缓冲区指针重新指向开头			
							break;//缓冲区慢时中止
						}
						else {
							Pre_Cluster=pfile->fileCluster;
							pfile->fileCluster=NextCluster(pfile->fileCluster);//如果缓冲区没满则继续读
							}
						}	
					pfile->fileCluster=Pre_Cluster;
				}
				pfile->filebufnum=pfile->filesize%Block_Size;//指针指向末尾
//				pfile->filesize=0;
				for(i=pfile->filebufnum;i<Block_Size;i++)
					pfile->Buffer[i]=0xff;
				break;
			default:
				return NULL;	//可能是创建模式，不能重名
			}
			return pfile;	//找到文件
		}
	}//

	if(OpenMode== (FILEMODE_WRITE|FILEMODE_CREATE) ){	//创建文件
		//如果没有找到同名文件则创建文件，
		//创建的时候先建立root目录的数据，在最后，Close的时候
		//把数据写入root，所以，创建文件以后如果掉电，因为
		//没有Close，所以，文件对root目录没有影响。----by threewater
		DPRINTK("\nCreating OSFILE %s !",filename1);

		pfile=(OSFILE*)OSMemGet(pFileMem,&err);
			
		if(pfile) {
			pfile->fileCluster=AllocateCluster(0);
			DPRINTK("\nThe new cluster is %d",pfile->fileCluster);
			pfile->filebufnum=0;
			pfile->fileCurpos=0;
			pfile->filemode=OpenMode;
		}
		else
			return NULL;
			
		if(pfile->fileCluster){
				//接下来在刚才找到的空根目录处创建文件项

				//by threewater----------->
				memcpy(FileSystemRoot[emptyrootpos].filename, filename1, 11);
				FileSystemRoot[emptyrootpos].Attribute=0x20;//Attribute
				FileSystemRoot[emptyrootpos].time=0x1612;//Create time
				FileSystemRoot[emptyrootpos].date=0x2ea2;//Create date
				FileSystemRoot[emptyrootpos].cluster=pfile->fileCluster;
				FileSystemRoot[emptyrootpos].filelength=0;
				/*
				databuff[j*32+11]=0x20;//Attribute
				databuff[j*32+22]=0x12;//d6;//Create time
				databuff[j*32+23]=0x16;//da;//MSB
				databuff[j*32+24]=0xa2;//ae;//Create date
				databuff[j*32+25]=0x2e;
				databuff[j*32+26]=(pfile->fileCluster&0x000000ff);//first cluster
				databuff[j*32+27]=((pfile->fileCluster&0x0000ff00)>>8);
				databuff[j*32+28]=0x00;//OSFILE length
				databuff[j*32+29]=0x0;	//by threewater
				databuff[j*32+30]=0;
				databuff[j*32+31]=0;*/
				pfile->filesize=0;
				pfile->rootpos=emptyrootpos;

				printk("\nCreat OSFILE %s Succeed!",filename1);
				return pfile;
		}
		else{
			//磁盘满
			return NULL;
		}
	}

	//没找到文件
	DPRINTK("\nFile %s is not found!",filename1);
	return NULL;
}

//add by threewater-------->
U8 DeleteOSFile(char filename[])
{

	int j;
	char filename1[12]={0};

	FormatFileName(filename1,filename);
	for(j=0;j<RootEntry && (FileSystemRoot[j].filename[0]!=0x00);j++){
		if(!MemcmpNoUpper(filename1, FileSystemRoot[j].filename,11)){	//如果找到某项符合条件
			FileSystemRoot[j].filename[0]=OSFILE_DELETEMARK;
			DeleteFATList(FileSystemRoot[j].cluster);
			WriteFATFromMem();// write to Flash

			WriteFileRootFromMem();

			printk("\nFile %s have been Deleted!",filename1);
			return TRUE;
		}
	}

	printk("\nFile %s is not exist!\n",filename1);
	return FALSE;
}

/****************************************
	查找指定文件名的文件，可以是8.3或者11模式
	如果找到文件返回文件在目录表中的项的位置，
	否则，返回-1，
*****************************************/
int FindOSFile(char filename[])
{
	int j;
	char filename1[12]={0};
	FormatFileName(filename1,filename);

	for(j=0;j<RootEntry && (FileSystemRoot[j].filename[0]!=0x00);j++){
		if(!MemcmpNoUpper(filename1, FileSystemRoot[j].filename,11)){	//如果找到某项符合条件
			return j;
		}
	}
	return -1;
}

/****************************************
	重命名文件
*****************************************/
U8 RenameOSFile(char fromname[], char toname[])
{
	char filename1[12]={0};
	int pos;
	FormatFileName(filename1,toname);
	
	pos=FindOSFile(fromname);

	if(pos<0)
		return FALSE;

	memcpy(FileSystemRoot[pos].filename, filename1, 11);

	WriteFileRootFromMem();
	return TRUE;
}

//<--------add by threewater
U32 ReadOSFile(OSFILE* pfile ,U8* ReadBuffer, U32 nReadbyte)
{
	U32 i,CurrentSector,SecNumOfClus,Current_Cluster;
	if(pfile->filemode!=FILEMODE_READ)
		return 0;
	
	for(i=0;i<nReadbyte;i++){
		if(pfile->filesize<=pfile->fileCurpos)
			return i;
		(*ReadBuffer)=pfile->Buffer[(pfile->fileCurpos++)%Block_Size];//
		ReadBuffer++;
		pfile->filebufnum++;//整个文件的读写文置指针
		
		if(pfile->filebufnum==Block_Size){//如果到了缓冲区末尾
			pfile->filebufnum=0;
//			Pre_Block=pfile->fileCluster;
			pfile->fileCluster=Current_Cluster=NextCluster(pfile->fileCluster);
			if((pfile->fileCluster==0xffff)||((pfile->fileCluster&0xfff)==0xfff)||(pfile->fileCluster==0xffffffff))//文件结束
				return i;

			CurrentSector=(Current_Cluster-2)*SecPerClus+FirstDataSec;
			for(SecNumOfClus=0;SecNumOfClus<SecPerClus;SecNumOfClus++){
				ReadPage(Begin_Cluster+(CurrentSector+SecNumOfClus)/PagePerClus,(CurrentSector+SecNumOfClus)%PagePerClus,databuff);
				memcpy(pfile->Buffer+pfile->filebufnum, databuff,512);
				pfile->filebufnum+=512;
				
				if(pfile->filebufnum>=Block_Size) {
					pfile->filebufnum=0;//缓冲区指针重新指向开头
					break;//缓冲区满时中止
					}
				}
		}
		
	}
	return i;
}

U32 GetPosOSFile(OSFILE* pfile)
{
	return pfile->fileCurpos;
 //     return pfile->fileCurpos%Block_Size;

}

U32 SeekOSFile(OSFILE* pfile ,U32 nCurPos)
{
	U32 i,CurrentSector,SecNumOfClus,Pre_Cluster;
//	if(pfile->filemode!=FILEMODE_READ)
//		return 0;

	if(nCurPos>pfile->filesize)//文件大小越界
		return pfile->fileCurpos;
	
	if((nCurPos>=Block_Size)&&(pfile->fileCurpos<Block_Size)){//跨Block 往后Seek
		pfile->filebufnum=0;
//		Pre_Block=pfile->fileCluster;
		pfile->fileCluster=NextCluster(pfile->fileCluster);

		if((pfile->fileCluster==0xffff)&&(pfile->fileCluster==0xfff)&&(pfile->fileCluster==0xffffffff))
			return i;

		CurrentSector=(pfile->fileCluster-2)*SecPerClus+FirstDataSec;


		for(SecNumOfClus=0;(SecNumOfClus<SecPerClus)&&((pfile->fileCurpos+pfile->filebufnum)<pfile->filesize);SecNumOfClus++){
			ReadPage(Begin_Cluster+(CurrentSector+SecNumOfClus)/PagePerClus,(CurrentSector+SecNumOfClus)%PagePerClus,databuff);
			memcpy(pfile->Buffer+pfile->filebufnum, databuff,512);
			pfile->filebufnum+=512;
			if((pfile->filebufnum>=Block_Size)&&((pfile->fileCurpos+pfile->filebufnum)>=pfile->filesize)) {
				pfile->filebufnum=0;//缓冲区指针重新指向开头			
				break;//缓冲区慢时中止
				}
			}
		pfile->filebufnum =nCurPos-Block_Size;
		pfile->fileCurpos =nCurPos;
		return nCurPos;
		
		}
	else if((nCurPos<Block_Size)&&(pfile->fileCurpos>=Block_Size)){//跨Block 往回Seek
		pfile->filebufnum=0;
		//查找上一个Cluster
		
		Pre_Cluster=NextCluster(FileSystemRoot[pfile->rootpos].cluster);
		while(pfile->fileCluster!=NextCluster(Pre_Cluster)) {Pre_Cluster=NextCluster(Pre_Cluster);}//往后找，直到打到当前簇的上一簇
		pfile->fileCluster=Pre_Cluster;	//找到了肖前簇的前一簇
		
		if((pfile->fileCluster==0xffff)&&(pfile->fileCluster==0xfff)&&(pfile->fileCluster==0xffffffff))
			return i;

		CurrentSector=(pfile->fileCluster-2)*SecPerClus+FirstDataSec;
		for(SecNumOfClus=0;SecNumOfClus<SecPerClus;SecNumOfClus++){
			ReadPage(Begin_Cluster+(CurrentSector+SecNumOfClus)/PagePerClus,(CurrentSector+SecNumOfClus)%PagePerClus,databuff);
			memcpy(pfile->Buffer+pfile->filebufnum, databuff,512);
			pfile->filebufnum+=512;
			if(pfile->filebufnum>=Block_Size) {
				pfile->filebufnum=0;//缓冲区指针重新指向开头			
				break;//缓冲区慢时中止
				}
			}
		pfile->filebufnum =nCurPos;
		pfile->fileCurpos =nCurPos;
		return nCurPos;

		
 		}
	//在Block内Seek
	else {
		pfile->filebufnum=nCurPos%Block_Size;
		pfile->fileCurpos=nCurPos;
		return nCurPos;
		}
	return 0;

}


U8 WriteOSFile(OSFILE* pfile, U8* WriteBuffer, U32 nWritebyte)
{
	int i=0,j=0;
	U32 Pre_Cluster,Current_Cluster;
//	if((pfile->filemode&0xf) !=FILEMODE_WRITE)
//		return FALSE;

	Current_Cluster=pfile->fileCluster;
	for(i=0;i<nWritebyte;i++){
//		pfile->Buffer[pfile->fileCurpos++]=*WriteBuffer;
//		printk("%d ",pfile->fileCurpos%Block_Size);
		pfile->Buffer[(pfile->fileCurpos++)%Block_Size]=*WriteBuffer;
		WriteBuffer++;
		pfile->filebufnum++;
	
		if(pfile->filebufnum>=Block_Size){	//超过一个block大小
			//by threewater-------->
//			Erase_FileCluster(Current_Block-1);//by threewater
			Erase_FileCluster(Current_Cluster);//by threewater
			for(j=0;j<SecPerClus;j++) WritePage(Begin_Cluster+(2*Current_Cluster-2)+j/PagePerClus,j%PagePerClus,&pfile->Buffer[j*512]);
			
			pfile->filebufnum=0;
			
			Pre_Cluster=pfile->fileCluster;
			pfile->fileCluster=Current_Cluster=NextCluster(pfile->fileCluster);
			if(((pfile->fileCluster&0xffff)==0xffff)||((pfile->fileCluster&0xfff)==0xfff)||(pfile->fileCluster==0xffffffff))
				pfile->fileCluster=Current_Cluster=AllocateCluster(Pre_Cluster);
			DPRINTK("\nPre is %d,Amc:%d",Pre_Cluster,Current_Cluster);

//			//满一簇,应该直接分配新的空间,sep 29 by frank
//			pfile->fileCluster=AllocateCluster(pfile->fileCluster);
			if(pfile->fileCluster==0) 
				{
				pfile->filesize+=i;	//by frank
				return FALSE;
				}

//			Write2MemFat(Pre_Block,Current_Block);//by frank,because the link have been established in AllocatedCluster
		}
			//<-----by threewater
	}
	if(pfile->fileCurpos>pfile->filesize) pfile->filesize+=nWritebyte;	//by threewater
//	pfile->filesize+=nWritebyte;	//by threewater
	return TRUE;
}

int CloseOSFile(OSFILE* pfile)
{
	int j;
	U32 Current_Cluster;
	switch(pfile->filemode&0xf){
	case FILEMODE_WRITE:
		Current_Cluster=pfile->fileCluster;
//		Erase_FileCluster(Current_Block-1);//by threewater
		Erase_FileCluster(Current_Cluster);//by frank
		//Current_Block was minused 1 in Erase_FileCluster
		for(j=0;j<SecPerClus;j++) WritePage(Begin_Cluster+(2*Current_Cluster-2)+j/PagePerClus,j%PagePerClus,&pfile->Buffer[j*512]);

		//写入root表<------add by threewater
		FileSystemRoot[pfile->rootpos].filelength=pfile->filesize;
//		printk("\nFilelength:%d",pfile->filesize);
		//写入的时候有同名文件，删除原文件剩余的Cluster
//		 DeleteFATList(NextCluster(pfile->fileCluster));
		//WriteFATFromMem();// write to Flash
		//<--------add by threewater
		WriteFATFromMem();// write to Flash
		WriteFileRootFromMem();
		break;
	default:
		break;
	}
	OSMemPut(pFileMem, pfile);
	//<--------add by threewater

	return TRUE;
}


//add by threeweter------------->
//得到指定位置的文件名（包括扩展名），文件位置自动下移
U8 GetNextFileName(U32 *filepos,char filename[])
{
	int j;

	for(j=*filepos; j<RootEntry; j++){
		if(FileSystemRoot[j].filename[0]==0x00)
			return FALSE;

		if(FileSystemRoot[j].filename[0]!=OSFILE_DELETEMARK){	//找到有效的文件项
			//memcpy(filename,&FileSystemRoot[j].filename,11);			
			memcpy(filename,&FileSystemRoot[j].filename,sizeof(char)*11);
			filename[11]=0;
			*filepos=j+1;
			return TRUE;
		}
	}
	return FALSE;
}

//得到指定位置的文件名（包括扩展名）和大小，文件位置自动下移
static U8 GetNextFNSize(U32 *filepos,char filename[],int *filesize)
{
	int j;

	for(j=*filepos; j<RootEntry; j++){
		if(FileSystemRoot[j].filename[0]==0x00)
			return FALSE;

		if(FileSystemRoot[j].filename[0]!=OSFILE_DELETEMARK){	//找到有效的文件项
			memcpy(filename,&FileSystemRoot[j].filename,11);
			filename[11]=0;

			if(filesize)
				*filesize=FileSystemRoot[j].filelength;

			*filepos=j+1;
			return TRUE;
		}
	}
	return FALSE;
}

//列出当前位置开始第一个指定扩展名的文件，如果没有，则返回FALSE
U8 ListNextFileName(U32 *filepos, char FileExName[],char filename[])
{
	char tmpfilename[11];
	for(;;){
		if(!GetNextFileName(filepos,tmpfilename))
			return FALSE;
		if(MemcmpNoUpper(tmpfilename+8,FileExName, 3)==0){
			strncpy(filename,tmpfilename,8);
			filename[8]=0;
			return TRUE;
		}
		else if(*filepos==512)
			return FALSE;
	}
}
//<--------add by threewater

int Init_FAT_Info(int AutoFormat)
{
	INT8U err;
	OSFILE *pfile;

	///////////////////////////////////////////////////
	//得到引导扇区所在扇区号，如果介质是不带分区的，则0扇区就是BootSector了。
	if(ReadPage(Begin_Cluster,0,databuff)==FAIL){
		printk("Failed to Read BPB\n");
		return FALSE;
	}

	if(memcmp(databuff, BPB_Data, sizeof(BPB_Data))!=0){	//分区表错误
		printk("File Partition Error!\n");

		if (AutoFormat==1)	//自动格式化
			return Format_Fat12Media();
		else
			return FALSE;
	}


	if(!((databuff[0]==0xeb)&&(databuff[1]==0x3c)&&(databuff[2]==0x90))){	//通过判断EB ?? 90来看是否已经是BPB了
		//带分区的介质
//		BootSector=databuff[454]+databuff[455]*256+databuff[456]*(256*256)+databuff[457]*(256*256*256);
		printk("\nFile System has error,BPB Sector have been broken.");

		pfile=(OSFILE*)OSMemGet(pFileMem,&err);//由于需要缓冲区，故创建一个指针

		ReadPage(0,0,databuff);
		memcpy(pfile->Buffer,databuff,512);
		Write2Flash(Begin_Cluster, BPB_Sector, BPB_Sector, pfile->Buffer);

		OSMemPut(pFileMem, pfile);

		ReadPage(Begin_Cluster,0,databuff);
		if((databuff[0]==0xeb)&&(databuff[1]==0x3c)&&(databuff[2]==0x90)) 
			printk("\nFile System has been repaired successfully.");

		}
	else BootSector=0;
	///////////////////////////////////////////////////

	////////////////////////////////////////////////
	//得到保留扇区数,总扇区数,总扇区数／每簇扇区数得到簇数，是FAT类型的依据
	ReadPage(Begin_Cluster,BootSector,databuff);
	RsdSector=databuff[14]+databuff[15]*256;
	SecPerClus=databuff[13];

	BytesPerSec=databuff[12]*256+databuff[11];

	TotalSector=(databuff[20]*256+databuff[19]);
	TotalCapacity=TotalSector*BytesPerSec;
	TotalCluster=TotalSector/SecPerClus;//FAT16的簇总数=扇区总数/每簇扇区数

//	SectorofFatSize=((databuff[22]+databuff[23]*256));	by threewater 2003-12-4
	SectorofFatSize=Fat_Sector_Num;

	RootEntry=(databuff[18]*256+databuff[17]);

	FirstDataSec=BootSector+RsdSector+(SectorofFatSize*2)+((RootEntry*32+(BytesPerSec-1))/BytesPerSec);

	if(TotalCluster>65525){  //FAT32的扇区总数和FAT表项长度
		FAT_TYPE=FAT32;

		if(TotalSector==0) TotalSector=(databuff[32]+databuff[33]*256+databuff[34]*256*256+databuff[35]*256*256*256);
		TotalCapacity=TotalSector*BytesPerSec;
		TotalCluster=TotalSector/SecPerClus;

		SectorofFatSize=(databuff[36]+databuff[37]*256+databuff[38]*256*256+databuff[39]*256*256*256);
		if(SectorofFatSize>(TotalCluster*16/512)) SectorofFatSize=((databuff[22]+databuff[23]*256));
		RootEntry=(databuff[44]*256+databuff[43]);
		FirstDataSec=BootSector+RsdSector+(SectorofFatSize*2)+((RootEntry*32+(BytesPerSec-1))/BytesPerSec);
		
		}
	else if((TotalCluster>0)&&(TotalCluster<4085)) {//FAT12
		FAT_TYPE=FAT12;
		}
	else {	//FAT16
		FAT_TYPE=FAT16;
		}

	ReadFAT2Mem();
	ReadFileRoot2Mem();
	return TRUE;
}

unsigned int NextCluster(unsigned int CurrentCluster)
{
	unsigned int i,j;
	//by threewater ---------------->
/*	if(FAT_TYPE==FAT12){
		ReadPage(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/SecPerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%SecPerClus,databuff);
		if(CurrentCluster%2==0) {
			j=databuff[(CurrentCluster*3/2)%BytesPerSec+1];
			i=databuff[(CurrentCluster*3/2)%BytesPerSec]|(j<<8);
			i&=0x0fff;
			}
		else {
			j=databuff[(CurrentCluster*3/2)%BytesPerSec+1];
			i=databuff[(CurrentCluster*3/2)%BytesPerSec]|(j<<8);
			i=(i>>4);
			}
		return (i);
		}
	else if(FAT_TYPE==FAT16){
		ReadPage(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*2)/BytesPerSec)/SecPerClus,(BootSector+RsdSector+(CurrentCluster*2)/BytesPerSec)%SecPerClus,databuff);
		i=databuff[(CurrentCluster*2)%BytesPerSec+1]*256+databuff[(CurrentCluster*2)%BytesPerSec];
		return (i);
		}
	else if(FAT_TYPE==FAT32){
		ReadPage(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*4)/BytesPerSec)/SecPerClus,(BootSector+RsdSector+(CurrentCluster*4)/BytesPerSec)%SecPerClus,databuff);
		i=databuff[(CurrentCluster*4)%BytesPerSec+3]*256*256*256+databuff[(CurrentCluster*4)%BytesPerSec+2]*256*256+databuff[(CurrentCluster*4)%BytesPerSec+1]*256+databuff[(CurrentCluster*4)%BytesPerSec];
		return (i);
		}*/
	if(CurrentCluster>=SectorofFatSize*BytesPerSec)
		return -1;

	if(FAT_TYPE==FAT12){
		if(CurrentCluster%2==0) {
			j=FileSystemFAT[(CurrentCluster*3/2)+1];
			i=FileSystemFAT[CurrentCluster*3/2]|(j<<8);
			i&=0x0fff;
		}
		else {
			j=FileSystemFAT[(CurrentCluster*3/2)+1];
			i=FileSystemFAT[CurrentCluster*3/2]|(j<<8);
			i=(i>>4);
		}
		return (i);
	}
	else if(FAT_TYPE==FAT16){
		i=FileSystemFAT[(CurrentCluster*2)+1]*256+FileSystemFAT[(CurrentCluster*2)];
		return (i);
	}
	else if(FAT_TYPE==FAT32){
		i=FileSystemFAT[(CurrentCluster*4)+3]*256*256*256+
			FileSystemFAT[(CurrentCluster*4)+2]*256*256+
			FileSystemFAT[(CurrentCluster*4)+1]*256+FileSystemFAT[(CurrentCluster*4)];
		return (i);
	}
	return -1;
	//<-------------------by threewater
}

unsigned int AllocateCluster(unsigned int CurrentCluster)
{
	unsigned int i=0,j=0,s=0;
	unsigned int FAT12_1,FAT12_2;
	switch(FAT_TYPE){
		
		case FAT12:
//				for(j=0;j<(512*SectorofFatSize-3);j+=3) {
				for(j=0;j<FAT_TABLE_SIZE;j+=3) {	//by threewater
					s=FileSystemFAT[j+1];FAT12_1=((s<<8)|FileSystemFAT[j]);FAT12_1&=0x0fff;
					s=FileSystemFAT[j+2];FAT12_2=(s<<8)|FileSystemFAT[j+1];FAT12_2=FAT12_2>>4;
//					printk("FAT12_1:%x,FAT12_2:%x",FAT12_1,FAT12_2);
					if(FAT12_1==0x00) {
						//或者接连两个0，则置第一个零为FF，第二个零的低位为F，且表项号为i*2/3,i为此字节的下标
						FileSystemFAT[j]=0xff;
						FileSystemFAT[j+1]|=0x0f;
//						printk("\nj1:%x,%x%x\n",j,databuff[j],databuff[j+1]);

//						memcpy(Buffer+(BootSector+RsdSector+i)*512,databuff,512);
//						memcpy(Buffer+(BootSector+RsdSector+i+SectorofFatSize)*512,databuff,512);
//						Write2Flash(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,(BootSector+RsdSector+i+SectorofFatSize)%PagePerClus,Buffer);

						FAT12_1=(j+i*512)*8/FAT_TYPE;
						if(CurrentCluster!=0) {//将原来的末簇指向新分配的簇
//							ReadPage(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/PagePerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%PagePerClus,databuff);
//							DPRINTK("\nCurrent:%x ",CurrentCluster);

							if(CurrentCluster%2==0) {
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec+1]&=0xf0;
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec+1]|=((FAT12_1&0x0f00)>>8);
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec]=FAT12_1&0x00ff;
								
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)*512,databuff,512);
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec+SectorofFatSize)*512,databuff,512);
//								Write2Flash(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/PagePerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%PagePerClus,(BootSector+RsdSector+i+SectorofFatSize)%PagePerClus,Buffer);


								}
							else {
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec+1]=(FAT12_1&0x0ff0)>>4;
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec]&=0x0f;
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec]|=((FAT12_1&0x000f)<<4);
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)*512,databuff,512);
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec+SectorofFatSize)*512,databuff,512);
//								Write2Flash(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/PagePerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%PagePerClus,(BootSector+RsdSector+i+SectorofFatSize)%PagePerClus,Buffer);
								}
//							printk("\n");for(s=0;s<64;s++) printk("%x ",databuff[s]);
							
						}
					
						return(FAT12_1);
						}
					else if((FAT12_1!=0x00)&&(FAT12_2==0x00)){
						//某个字节为0，且其前一字节高位为0时，此FAT表项为空，置前一字节高位为F，本字节为FF，且表项号为i*2/3,i为此字节的下标
						FileSystemFAT[j+2]=0xff;
						FileSystemFAT[j+1]|=0xf0;
//						printk("j2:%x,%x%x\n",j,databuff[j+2],databuff[j+1]);

//						memcpy(Buffer+(BootSector+RsdSector+i)*512,databuff,512);
//						memcpy(Buffer+(BootSector+RsdSector+i+SectorofFatSize)*512,databuff,512);
//						Write2Flash(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,(BootSector+RsdSector+i+SectorofFatSize)%PagePerClus,Buffer);

						FAT12_2=(j+i*512+2)*8/FAT_TYPE;
						if(CurrentCluster!=0) {//将原来的末簇指向新分配的簇
//							ReadPage(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/PagePerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%PagePerClus,databuff);
//							DPRINTK("Allocate:%x ",CurrentCluster);
							
							if(CurrentCluster%2==0) {
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec+1]&=0xf0;
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec+1]|=((FAT12_2&0x0f00)>>8);
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec]=FAT12_2&0x00ff;
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)*512,databuff,512);
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec+SectorofFatSize)*512,databuff,512);
//								Write2Flash(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/PagePerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%PagePerClus,(BootSector+RsdSector+i+SectorofFatSize)%PagePerClus,Buffer);
								}
							else {
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec+1]=(FAT12_2&0x0ff0)>>4;
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec]&=0x0f;
								FileSystemFAT[(CurrentCluster*3/2)%BytesPerSec]|=((FAT12_2&0x000f)<<4);
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)*512,databuff,512);
//								memcpy(Buffer+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec+SectorofFatSize)*512,databuff,512);
//								Write2Flash(Begin_Cluster+(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)/PagePerClus,(BootSector+RsdSector+(CurrentCluster*3/2)/BytesPerSec)%PagePerClus,(BootSector+RsdSector+i+SectorofFatSize)%PagePerClus,Buffer);
								}
//							printk("\n");for(s=0;s<64;s++) printk("%x ",databuff[s]);
							
						}
						
						return(FAT12_2);
					}
				}
			break;
		case FAT16:
			for(i=0;i<SectorofFatSize;i++)	{
				ReadPage(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,databuff);
		//		for(s=0;s<512;s++) printk("%x",databuff[0x80+s]);
				for(j=0;j<512/(FAT_TYPE/8);j++) {
					if((databuff[j*(FAT_TYPE/8)]==0x00)&&(databuff[j*(FAT_TYPE/8)+1]==0x00)) {
						databuff[j*(FAT_TYPE/8)]=0xff;
						databuff[j*(FAT_TYPE/8)+1]=0xff;

//						memcpy(Buffer+(BootSector+RsdSector+i)*512,databuff,512);
//						Write2Flash(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,(BootSector+RsdSector+i)%PagePerClus,Buffer);
						
						return(j+i*512/(FAT_TYPE/8));
						}
					}
				}
			break;
		case FAT32:
			for(i=0;i<SectorofFatSize;i++)	{
				ReadPage(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,databuff);
		//		for(s=0;s<512;s++) printk("%x",databuff[0x80+s]);
				for(j=0;j<512/(FAT_TYPE/8);j++) {
					if((databuff[j*(FAT_TYPE/8)]==0x00)&&(databuff[j*(FAT_TYPE/8)+1]==0x00)&&(databuff[j*(FAT_TYPE/8)+2]==0x00)&&(databuff[j*(FAT_TYPE/8)+3]==0x00)) {
						databuff[j*(FAT_TYPE/8)]=0xff;
						databuff[j*(FAT_TYPE/8)+1]=0xff;
						databuff[j*(FAT_TYPE/8)+2]=0xff;
						databuff[j*(FAT_TYPE/8)+3]=0xff;

//						memcpy(Buffer+(BootSector+RsdSector+i)*512,databuff,512);
//						Write2Flash(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,(BootSector+RsdSector+i)%PagePerClus,Buffer);
						
						return(j+i*512/(FAT_TYPE/8));
						}
					}
				}
			
			break;
	default:break;
	}
	return 0;
}

/*
unsigned int AllocateCluster(OSFILE *pfile)
{
	unsigned int i,j,s;
	unsigned int FAT12_1,FAT12_2;
	switch(FAT_TYPE){
		
		case FAT12:
				//by threewater--------------------->
				for(j=0;j<(512-3)*SectorofFatSize;j+=3) {
					s=FileSystemFAT[j+1];	FAT12_1=((s<<8)|FileSystemFAT[j]);	FAT12_1&=0x0fff;
					s=FileSystemFAT[j+2];	FAT12_2=(s<<8)|FileSystemFAT[j+1];	FAT12_2=FAT12_2>>4;
//					DPRINTK("FAT12_1:%x,FAT12_2:%x",FAT12_1,FAT12_2);
					if(FAT12_1==0x00) {
						//或者接连两个0，则置第一个零为FF，第二个零的低位为F，且表项号为i*2/3,i为此字节的下标
						FileSystemFAT[j]=0xff;
						FileSystemFAT[j+1]|=0xf;//by threewater
//						FileSystemFAT[j+1]=0x0f;
//						DPRINTK("\nj1:%x,%x%x\n",j,databuff[j],databuff[j+1]);
						return(j*8/FAT_TYPE);
					}
					else if((FAT12_1!=0x00)&&(FAT12_2==0x00)){
						//某个字节为0，且其前一字节高位为0时，此FAT表项为空，置前一字节高位为F，本字节为FF，且表项号为i*2/3,i为此字节的下标
						FileSystemFAT[j+2]=0xff;
						FileSystemFAT[j+1]|=0xf0;
						return((j+2)*8/FAT_TYPE);
					}
				}
				//<---------------------by threewater
			break;
		case FAT16:
			for(i=0;i<SectorofFatSize;i++)	{
				ReadPage(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,databuff);
		//		for(s=0;s<512;s++) DPRINTK("%x",databuff[0x80+s]);
				for(j=0;j<512/(FAT_TYPE/8);j++) {
					if((databuff[j*(FAT_TYPE/8)]==0x00)&&(databuff[j*(FAT_TYPE/8)+1]==0x00)) {
						databuff[j*(FAT_TYPE/8)]==0xff;
						databuff[j*(FAT_TYPE/8)+1]==0xff;

						memcpy(pfile->Buffer+(BootSector+RsdSector+i)*512,databuff,512);
						Write2Flash(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,(BootSector+RsdSector+i)%PagePerClus,pfile->Buffer);
						
						return(j+i*512/(FAT_TYPE/8));
						}
					}
				}
			break;
		case FAT32:
			for(i=0;i<SectorofFatSize;i++)	{
				ReadPage(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,databuff);
		//		for(s=0;s<512;s++) DPRINTK("%x",databuff[0x80+s]);
				for(j=0;j<512/(FAT_TYPE/8);j++) {
					if((databuff[j*(FAT_TYPE/8)]==0x00)&&(databuff[j*(FAT_TYPE/8)+1]==0x00)&&(databuff[j*(FAT_TYPE/8)+2]==0x00)&&(databuff[j*(FAT_TYPE/8)+3]==0x00)) {
						databuff[j*(FAT_TYPE/8)]==0xff;
						databuff[j*(FAT_TYPE/8)+1]==0xff;
						databuff[j*(FAT_TYPE/8)+2]==0xff;
						databuff[j*(FAT_TYPE/8)+3]==0xff;

						memcpy(pfile->Buffer+(BootSector+RsdSector+i)*512,databuff,512);
						Write2Flash(Begin_Cluster+(BootSector+RsdSector+i)/PagePerClus,(BootSector+RsdSector+i)%PagePerClus,(BootSector+RsdSector+i)%PagePerClus,pfile->Buffer);
						
						return(j+i*512/(FAT_TYPE/8));
						}
					}
				}
			
			break;
	default:break;
	}
	return 0;
}
*/

void TestFAT_READ(char filename[])
{
	OSFILE *file2;
	int i;
//	char LineString[100]={0,};

	file2=OpenOSFile(filename,FILEMODE_READ);
	if(file2!=NULL)
		for(i=0;i<16;i++) DPRINTK("%c",file2->Buffer[i]);
	/*
	j=LineReadOSFile(file2, LineString);
	DPRINTK("LINE1\n");
	for(i=0;i<j;i++) DPRINTK("%c",LineString[i]);
	j=LineReadOSFile(file2, LineString);
	DPRINTK("LINE2\n");
	for(i=0;i<j;i++) DPRINTK("%c",LineString[i]);
	j=LineReadOSFile(file2, LineString);
	DPRINTK("LINE3\n");
	for(i=0;i<j;i++) DPRINTK("%c",LineString[i]);

	SeekOSFile(file2,GetPosOSFile(file2));
	j=LineReadOSFile(file2, LineString);
	DPRINTK("LINE4\n");
	for(i=0;i<j;i++) DPRINTK("%c",LineString[i]);
	j=LineReadOSFile(file2, LineString);
	DPRINTK("LINE5\n");
	for(i=0;i<j;i++) DPRINTK("%c",LineString[i]);
	j=LineReadOSFile(file2, LineString);
	DPRINTK("LINE6\n");
	for(i=0;i<j;i++) DPRINTK("%c",LineString[i]);
	*/
}
void TestFAT_DELETE(char filename[])
{
	DeleteOSFile(filename);

}
void TestFAT_CREATE(char filename[])
{
	OSFILE *file2;
	int i;
	file2=OpenOSFile(filename,FILEMODE_WRITE);
	for(i=0;i<1024;i++) file2->Buffer[i]='A'+i/40;
	for(i=0;i<1024;i+=40) {file2->Buffer[i]=13;file2->Buffer[i+1]=10;}
	CloseOSFile(file2);

}
void TestFAT_COPY(char filename1[],char filename2[])
{
	OSFILE *file1,*file2;
	int i;
	file1=OpenOSFile(filename1,FILEMODE_READ);
	file2=OpenOSFile(filename2,FILEMODE_WRITE);
	for(i=0;i<file1->filesize;i++) file2->Buffer[i]=file1->Buffer[i];
	CloseOSFile(file1);
	CloseOSFile(file2);
}

//获得文件分区信息，可以为空指针，返回块大小
int GetFS_Info(int *TotalSec, int *unusedSec, int *badSec)
{
	int unused=0, bad=0,i;
	unsigned int FAT12_1, FAT12_2, s;
	if(TotalSec)
		*TotalSec=TotalSector;

	if(!unusedSec && !badSec)
		return BytesPerSec;

	//计算未使用的块和坏块
	for(i=0;i<(TotalSector*3/2/SecPerClus-3); i+=3){
		s=FileSystemFAT[i+1];
		FAT12_1=((s<<8)|FileSystemFAT[i]);
		FAT12_1&=0x0fff;
		s=FileSystemFAT[i+2];
		FAT12_2=(s<<8)|FileSystemFAT[i+1];
		FAT12_2=FAT12_2>>4;

		if(FAT12_1==0)	//unused
			unused++;
		if(FAT12_2==0)	//unused
			unused++;
	}

	if(unusedSec)
		*unusedSec=unused*SecPerClus;

	if(badSec)
		*badSec=bad;

	return BytesPerSec;
}

#else	//for yaffs file system

int initOSFile()
{
	ucos2_malloc_init();

	yaffs_StartUp();

	return TRUE;
}

#endif	//#if (USE_YAFFS==0)

