#include "consoleshell.h"
//#include "linuxpatch.h"
int main(int argc, char **argv){
	//  ctSetup();
		tty_init();//��ʼ�����ڵ�ͨѶ����
		consoleshell();
		tty_end();//�ָ����ĵĴ�������
	  return 0;
}
