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
#include "sensor.h"

main(int argc, char* argv[]){
	tty_init();
	consoleshell();
	tty_end();
	printf("exit\n");
	exit(0); 
}
