#include "consoleshell.h"

int main(int argc, char **argv){

		tty_init();//��ʼ�����ڵ�ͨѶ����
		consoleshell();
		tty_end();//�ָ����ĵĴ�������
	  return 0;
}
