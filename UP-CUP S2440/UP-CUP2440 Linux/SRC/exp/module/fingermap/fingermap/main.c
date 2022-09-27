#include "consoleshell.h"

int main(int argc, char **argv){

		tty_init();//初始化串口的通讯设置
		consoleshell();
		tty_end();//恢复更改的串口设置
	  return 0;
}
