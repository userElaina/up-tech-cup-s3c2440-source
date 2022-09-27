#ifndef _SENSORPROCESS_H_
#define _SENSORPROCESS_H_

#define SensorLED4_EN          7  // PC.7 - Output: Red LED
#define SensorLED3_EN          6  // PC.6 - Output: Red LED
#define SensorLED2_EN          5  // PC.5 - Output: Red LED
#define SensorLED1_EN          4  // PC.4 - Output: Red LED


// Clears SensorLED
#define CLR_SensorLED4_EN()                  (PORTC |= BM(SensorLED4_EN))
#define CLR_SensorLED3_EN()                  (PORTC |= BM(SensorLED3_EN))
#define CLR_SensorLED2_EN()                  (PORTC |= BM(SensorLED2_EN))
#define CLR_SensorLED1_EN()                  (PORTC |= BM(SensorLED1_EN))


// Set SensorLED
#define SET_SensorLED4()                  (PORTC &= ~BM(SensorLED4_EN))
#define SET_SensorLED3()                  (PORTC &= ~BM(SensorLED3_EN))
#define SET_SensorLED2()                  (PORTC &= ~BM(SensorLED2_EN))
#define SET_SensorLED1()                  (PORTC &= ~BM(SensorLED1_EN))

#define VREF 					3.3
#define LightResistance 		0
#define TemperatureResistance 	1
#define MIC 					2
#define HumiditySensor 			3
#define Potantiometers 			4
#define Temperature18b20 		5


void SensorPort_Init(void);
void ADC_Init(void); 
float StartADC( char SensorCode );

void Timer2_Init(void);
void INT4_Init(void);
void ADC_Test(unsigned int ADC_NUM);
unsigned int GetSensorInformation( unsigned char cmd );

#endif 


