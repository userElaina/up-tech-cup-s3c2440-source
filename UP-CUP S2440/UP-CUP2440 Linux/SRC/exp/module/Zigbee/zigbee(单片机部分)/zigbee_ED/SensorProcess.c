#include "SensorProcess.h"
#include "18b20.h"
#include "zigbee.h"		//Zigbee定义
#include "zAPL.h"	//Zigbee应用层


char volatile SoundFlag = 0;           //MIC检测声音标志     

unsigned char HumidityCompleteFlag = 0;

unsigned int RH = 0;
unsigned int TIMER2_H = 0;
unsigned int FOUT = 0;//湿度传感器检测到信号的频率 unit: Hz
	
void SensorPort_Init(void)
{
	DDRC = 0xF0;
	PORTC |= 0xF0;
	DDRD &= ~0x80;
	DDRE &= ~0x10;
	DDRF &= ~0x07;
}

/*********************************************************************
 * Function:        void ADC_Init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:     初始化 ADC，但不使能
 *                     ADC0:电位器 ADC1:热敏电阻 ADC2:光敏电阻
 *                     Conversion time: 3uS
 * Note:            None
 ********************************************************************/
void ADC_Init(void)
{  
	ADCSRA = 0x00; //disable adc
    ADMUX = 0x00; //0x00:adc0  0x01:adc1  0xo2:adc2
    ACSR  = 0x80;
    ADCSRA = 0x01;
}   

/*********************************************************************
 * Function:        float StartADC( char SensorCode )
 *
 * PreCondition:    None
 *
 * Input:           char SensorCode--0:电位器 1:热敏电阻 2:光敏电阻
 *
 * Output:          float voltage
 *
 * Side Effects:    None
 *
 * Overview:     初始化 ADC，但不使能
 *                     Conversion time: 3uS
 * Note:            None
 ********************************************************************/
float StartADC( char SensorCode )
{  
    float voltage = 0;
	ADCSRA = 0x00; //disable adc
	ADMUX = 0x00|SensorCode;
	ADCSRA = 0xc1;
	while( ADCSRA&0x40 );
	voltage = (float)ADC*VREF/1024;
	return voltage;
} 

void ADC_Test(unsigned int ADC_NUM)
{
	float v;
	unsigned int i = 0;
	ADC_Init();
	while(1)
	{
		switch(ADC_NUM)
		{
			case 0:{v = StartADC(0);break;}
			case 1:{v = StartADC(1);break;}
			case 2:{v = StartADC(2);break;}
			default:break;
		}
		i = (unsigned int)(VREF / v);
		switch(i)
		{
			case 1:
				{
					SET_SensorLED1();
					break;
				}

			case 2:
				{
					SET_SensorLED2();
					break;
				}

			case 3:
				{
					SET_SensorLED3();
					break;
				}

			default:
				{
					SET_SensorLED4();
					break;
				}
		}
		i=5000;
		while(i--);
		CLR_SensorLED4_EN();
 		CLR_SensorLED3_EN();
		CLR_SensorLED2_EN();
		CLR_SensorLED1_EN();
		
	}
}


/*********************************************************************
 * Function:        void INT4_Init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:      None
 *                  
 * Note:            None
 ********************************************************************/
void INT4_Init(void)
{
	cli();
	SoundFlag = 0;
	//每次检测开启时初始化标志位
	//SET_SensorLED1();//~~~
	EIMSK = 0x00;
	EICRB |= 0x03; //extended ext ints
	EIMSK |= 0x10;
	sei();
}

SIGNAL(INT4_vect)
{
	int i = 1000;
	//SET_SensorLED2();//~~~
	while( i-- );
	if( PINE&0x10 )
	{
		SoundFlag = 1;
		/*
		//SensorLED1亮，用于测试
		
		SET_SensorLED1();
		i = 1000;
		while( i-- );
		CLR_SensorLED1_EN();
		*/
	}
}

void Timer3_Init(void)
{
	HumidityCompleteFlag = 0;

	//SET_SensorLED1();
	TIMSK |= 0x01; //timer interrupt sources
 	ETIMSK |= 0x04; //extended timer interrupt sources

	TCCR3B = 0x00; //stop
	TCNT3H = 0xE3; //setup
	TCNT3L = 0xE1;
	OCR3AH = 0x1C;
	OCR3AL = 0xFF;
	OCR3BH = 0xFF;
	OCR3BL = 0xFF;
	OCR3CH = 0xFF;
	OCR3CL = 0xFF;
	ICR3H = 0x1C;
	ICR3L = 0x1F;
	TCCR3A = 0x00;

	Timer2_Init();
	TCCR3B = 0x05; //start Timer
}

SIGNAL(TIMER3_OVF_vect)
{
	double x1=0;
	double x2=0;
	double x3=0;
	//unsigned int i=0;
	TCCR2 = 0x00;
	TCCR3B = 0x00;	
	//SET_SensorLED4();
	FOUT = TIMER2_H*256+TCNT2;
	//6216 = 24*256+72
	//R10虚拟值为619.7K
	
	// notice 
	x1=1000000000/( (double)FOUT*893.75*180 );
	x2=x1*x1;
	x3=x2*x1;
	RH=(unsigned int)(-3.4656*1000*x3+1.0732*10000*x2-1.0457*10000*x1+3.2459*1000);
	if(RH>100)RH=100;
	if(RH<0.01)RH=0;
	/*
	switch(RH/10-4)
	{
		case 0:
			{
				break;
			}
		case 1:
			{
				SET_SensorLED1();
				break;
			}
		case 2:
			{
				SET_SensorLED2();
				break;
			}
		case 3:
			{
				SET_SensorLED3();
				break;
			}
		case 4:
			{
				SET_SensorLED4();
				break;
			}
		default:
			{
				SET_SensorLED1();
				SET_SensorLED2();
				SET_SensorLED3();
				SET_SensorLED4();
				break;
			}
	}
	
	i=5000;
	while(i--);
	CLR_SensorLED4_EN();
 	CLR_SensorLED3_EN();
	CLR_SensorLED2_EN();
	CLR_SensorLED1_EN();
	*/
	TIMER2_H = 0;
	HumidityCompleteFlag = 1;
	//测试用
	//Timer3_Init();
}

/*********************************************************************
 * Function:        void Timer2_Init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:      用于检测湿度传感器的频率信号，用作计数器
 *                  
 * Note:            None
 ********************************************************************/

void Timer2_Init(void)
{  
	//SET_SensorLED3();
	TIMSK |= 0x80;
	TCCR2 = 0x00; 
 	TCNT2 = 0x00; 
 	OCR2  = 0xFF;
 	TCCR2 = 0x07; 	
} 

SIGNAL(TIMER2_COMP_vect)
{
	TIMER2_H++;
	Timer2_Init();
	//SET_SensorLED3();
}


//传感器数据采集对外接口函数。目前得到的数据为经过初步
//处理的数据，不适合直接向coord   发送，须经过一定的处理进
//行类型转换。此函数还须加上发送数据相关程序。
unsigned int GetSensorInformation( unsigned char cmd )
{
	float voltage = 0;
	unsigned int i, j,k;
	float Temp18b20;
	unsigned int result = 0;
	switch(cmd)
	{
		//光敏电阻
		case LightResistance:
			{
				ADC_Init();
				voltage = StartADC(2);
				if(voltage<0.5)
				{
					result = 0;
					//暗
				}
				else if(voltage>2.5)
				{
					result = 2;
					//亮
				}
				else
				{
					result = 1;
					//正常
				}
				break;
			}

		//热敏电阻
		case TemperatureResistance:
			{
				ADC_Init();
				voltage = StartADC(1);
				if(voltage<1.5)
				{
					result = 0;					
					//冷
				}
				else if(voltage>1.9)
				{
					result = 2;
					//热
				}
				else
				{
					result = 1;
					//正常
				}
				break;	
			}

		//麦克
		case MIC:
			{
				/*
				INT4_Init();
				//SET_SensorLED3();//~~~
				i = 1000;
				j = 1000;
				while( i-- )
				{
					while(j--);
					j = 1000;
				}
				//等待一段时间，若有声音，进入中断处理程序，改变标志位
				cli();
				EIMSK &= ~0x10;
				sei();
				*/
				/*i = 65535;
				SoundFlag = 0;
				while(1)
				{
					if( PINE&0x10 )
					{
						j = 100;
						while(j--);
						if( PINE&0x10 )
						{
							SoundFlag = 1;
							break;
						}
					}
					i--;
					if( i == 0 )
					{
						SoundFlag = 0;
						break;
					}
					
				}*/
				SoundFlag = 0;
				for( i = 0;i<100;i++ )
				{
					for(j = 0;j<1000;j++)
					{
						if( PINE&0x10 )
						{
							k = 100;
							while(k--);
							if( PINE&0x10 )
							{
								SoundFlag = 1;
							}
						}
					}
				}
				

				//SET_SensorLED4();//~~~
				result = SoundFlag;
				//1有声，0无声
				
				//关闭中断
				break;	
			}

		//湿度传感器
		case HumiditySensor:
			{
				Timer3_Init();
				while(!HumidityCompleteFlag);
				result = RH;
				break;	
			}

		//电位器
		case Potantiometers:
			{
				ADC_Init();
				voltage = StartADC(0);
				result = (unsigned int)((voltage/VREF)*10000);
				//单位: 欧姆
				break;	
			}

		//18b20温度传感器
		case Temperature18b20:
			{
				Temp18b20 = Get_Temperature();//float型
				result = (unsigned int)(Temp18b20*10);
				//单位:0.1 摄氏度
				break;	
			}

		default:
			{
				break;
			}
	}
	return result;
}






