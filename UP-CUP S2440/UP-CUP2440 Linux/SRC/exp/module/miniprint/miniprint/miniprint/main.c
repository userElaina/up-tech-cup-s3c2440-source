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
	tty_init();//初始化串寇
	consoleshell();//检测用户命令并打印
  tty_end();//恢复串口的原本设置
	printf("\n$$$$$$$$$$$$$$$$$$$$\n");
  exit(0); 
}
