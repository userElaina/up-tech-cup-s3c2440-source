#ifndef  _SENSOR_H_
#define _SENSOR_H_

//18b20温度传感器
#define Temperature18b20	0x10

//MIC声音传感器
#define MIC					0x11

//触摸板
#define TouchPad			0x12

//热敏电阻
#define TemperatureResister 0x13

//光敏电阻
#define LightResister		0x14

//差分电压
#define DifferentialVoltage 0x15

//霍尔线性传感器
#define AH3503				0x16

//机械抖动开关
#define MOVESW 				0x17

//干簧管
#define REEDSW				0x18

//热释红外传感器
#define RE200B				0x19

//广谱气体传感器
#define MQ2					0x1A

//红外对射开关（打开）
#define StartInfraredRadio	0x1B

//红外对射开关（关闭）
#define FinishInfraredRadio	0x1C

//湿度传感器HS1101
#define HS1101				0x1D

//霍尔开关
#define AH3020				0x1E

//电压输入
#define VoltageInput		0x1F

//电流输入
#define CurrentInput		0x20


#define SET_BUZZER (PORTC |= 0x40)
#define CLR_BUZZER (PORTC &= ~0x40)

#define SET_INTERRUPT (PORTD &= ~0x80)
#define CLR_INTERRUPT (PORTD |= 0x80)

extern unsigned char CMD;


void GetSensorMessage( unsigned char param );

#endif 

