#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int MAX_LEN=96;			/* 定义读写的字节数 */
int main()
{  int fd,i;			/* fd为将要用到的文件描述符 */
   char buf[MAX_LEN],readic[MAX_LEN];
   for(i=0;i<MAX_LEN;i++){	/* 给数组赋值 */
     buf[i]=i+32;
   }
   
   if((fd=open("/dev/Mega8-icc",O_RDWR))==-1){ /* 打开IC卡设备,如果出错*/
     fprintf(stderr,"Open error %d\n",errno);        /* 打印errno变量的值 */
     exit(1);
   }
 
   printf("Write %d bytes data to /dev/Mega8-icc \n",MAX_LEN);
   if(write(fd,buf,MAX_LEN)==-1){                    /* 将数据写入IC卡 */
     fprintf(stderr,"Write error %d\n",errno);
     exit(1);
   }
    
   if((lseek(fd,0,SEEK_SET))==-1){           /* 将文件读写指针定位到起始位置 */
     fprintf(stderr,"Lseek error %d\n",errno);
     exit(1);
   }
   
   printf("\nRead %d bytes data from /dev/Mega8-icc \n",MAX_LEN);
   if(read(fd,readic,MAX_LEN)==-1){       /* 读IC卡,并将数据存入readic[]数组 */
     fprintf(stderr,"Read error %d\n",errno);
     exit(1);
   }
   
   if(close(fd)==-1){		/* 关闭IC卡设备 */
     fprintf(stderr,"Close error%d\n",errno);
     exit(1);
   }
   
   for(i=0;i<MAX_LEN;i++){	/* 输出显示读出的数据 */
     if((i%8)==0)
       printf("\n");
     printf("%4c",readic[i]);
   }
   printf("\n");
   
   return 0;			/* 成功返回,程序结束 */
}
