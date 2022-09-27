#ifndef  _SENSOR_H_
#define _SENSOR_H_

//18b20�¶ȴ�����
#define Temperature18b20	0x10

//MIC����������
#define MIC					0x11

//������
#define TouchPad			0x12

//��������
#define TemperatureResister 0x13

//��������
#define LightResister		0x14

//��ֵ�ѹ
#define DifferentialVoltage 0x15

//�������Դ�����
#define AH3503				0x16

//��е��������
#define MOVESW 				0x17

//�ɻɹ�
#define REEDSW				0x18

//���ͺ��⴫����
#define RE200B				0x19

//�������崫����
#define MQ2					0x1A

//������俪�أ��򿪣�
#define StartInfraredRadio	0x1B

//������俪�أ��رգ�
#define FinishInfraredRadio	0x1C

//ʪ�ȴ�����HS1101
#define HS1101				0x1D

//��������
#define AH3020				0x1E

//��ѹ����
#define VoltageInput		0x1F

//��������
#define CurrentInput		0x20


#define SET_BUZZER (PORTC |= 0x40)
#define CLR_BUZZER (PORTC &= ~0x40)

#define SET_INTERRUPT (PORTD &= ~0x80)
#define CLR_INTERRUPT (PORTD |= 0x80)

extern unsigned char CMD;


void GetSensorMessage( unsigned char param );

#endif 

