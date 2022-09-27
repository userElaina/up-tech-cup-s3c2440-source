/************************************************
 *  	zigbee Demo for UP-CUP6410-II
 *  	use ttySAC1 to ctrol zigbee coor
 *  	by lyj_uptech@126.com   
 *	20070316
 *   	updated by sprife
 * 	2009-12-14
*************************************************/
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "tty.h"
#include "zigbee.h"

main(int argc, char* argv[]){
	tty_init();//初始化串口
	consoleshell();//检测用户命令并打印
  	tty_end();//恢复串口的原本设置
  	exit(0); 
}
