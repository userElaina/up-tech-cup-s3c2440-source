/************************************************
 * miniprint Demo
 *  use ttys1 to ctrol miniprint
 *  by lyj_uptech@126.com   
 * 20070316
 *   See to iccard
*************************************************/
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "tty.h"
#include "miniprint.h"

main(int argc, char* argv[]){
	tty_init();//��ʼ������
	consoleshell();//����û������ӡ
  tty_end();//�ָ����ڵ�ԭ������
	printf("\n$$$$$$$$$$$$$$$$$$$$\n");
  exit(0); 
}
