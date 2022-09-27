#include "miniprint.h"

//��ӡ�ո�
void print_space(uchar n){
	uchar txbuff[3];
	set_chinese_enter();
	txbuff[0]=0x1b;
	txbuff[1]=0x66;
	txbuff[2]=0x00;
	txbuff[3]=n;
	tty_write(txbuff,4);
	set_chinese_end();
}
//�ƶ�һ��
void enter(){
	unsigned char buff=0x0a;
	tty_write(&buff,1);
}
//�ƶ�n��
//pram int n
void print_move_n(int n){
	unsigned char buff=10;
	tty_write(&buff,1);
}
//��ӡ����
void print_chinese(uchar *data,uchar n){
	set_chinese_enter();
//	set_extend(n);
	while((*data)!=0){
	    tty_write(data,1);
	    data ++;
	 }
	set_chinese_end();
	printf("\n print over");
}
//��ӡ
void print(uchar *data,uchar n){
	set_chinese_end();
	set_extend(n);
	while((*data)!=0){
	    tty_write(data,1);
	    data ++;
	 }
	printf("\n print over");
}
//������߽�
void set_band_left(uchar n){
	uchar txbuff[3];	
	txbuff[0]=0x1b;
	txbuff[1]=0x6c;
	txbuff[2]=n;
	tty_write(txbuff,3);
}
//�����ұ߽�
void set_band_right(uchar n){
	uchar txbuff[3];	
	txbuff[0]=0x1b;
	txbuff[1]=0x51;
	txbuff[2]=n;
	tty_write(txbuff,3);
}
//���뺺�ִ�ӡģʽ
void set_chinese_enter(){
	uchar txbuff[5];	
	txbuff[0]=0x1c;
	txbuff[1]=0x26;
	txbuff[2]=0x1b;
	txbuff[3]=0x38;
	txbuff[4]=0x01;
	tty_write(txbuff,5);
}
//�˳����ִ�ӡģʽ
void set_chinese_end(){
	uchar rxbuff[3];
	rxbuff[0]=0x1c;
	rxbuff[1]=0x2e;
	rxbuff[2]=0x0d;
	tty_write(rxbuff,3);
}
//���ֺ���Ŵ�
void set_chinese_extend_1(uchar n){
	uchar txbuff[3];	
	txbuff[0]=0x1b;
	txbuff[1]=0x55;
	txbuff[2]=n;
	tty_write(txbuff,3);
}
void set_chinese_extend_2(uchar n){
	uchar txbuff[3];
	txbuff[0]=0x1b;
	txbuff[1]=0x56;
	txbuff[2]=n;
	tty_write(txbuff,3);
}
//���ֺ�������Ŵ�
void set_extend(uchar n){
	uchar txbuff[3];	
	txbuff[0]=0x1b;
	txbuff[1]=0x57;
	txbuff[2]=n;
	tty_write(txbuff,3);
}

//���ù����������ʽ
void set_EAN_h(){
	//����������߶�
	uchar txbuff[3];	
	txbuff[0]=0x1d;
	txbuff[1]=0x68;
	txbuff[2]=0xff;
	tty_write(txbuff,3);
}
//�����Ƿ��ӡHRI�ַ�
void set_EAN_HRI(){
	uchar txbuff[3];	
	txbuff[0]=0x1d;
	txbuff[1]=0x48;
	txbuff[2]=0x01;
	tty_write(txbuff,3);
}
//���ڲ��õ��ǵ͹��ĵ��豸������û�������
//��ӡ������
void print_EAN(){
	uchar txbuff[5];
	txbuff[0]=0x1d;
	txbuff[1]=0x6b;
	txbuff[2]=0x02;//EAN-13
	txbuff[3]=0xef;//��ӡ�ַ�
	txbuff[4]=0x00;
//	set_EAN_HRI();
//	set_EAN_h();
	tty_write(txbuff,5);
}
