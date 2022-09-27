#include "Generic.h"

unsigned char CMD = 0x00;

void GetSensorMessage( unsigned char param )
{
	float result = 0;
	unsigned char resultH = 0;
	unsigned char resultL = 0;
	unsigned int i,j,k;
	unsigned char SoundFlag = 0;
	unsigned char TouchFlag = 0;
	unsigned char MoveFlag = 0;
	unsigned char REEDFlag = 0;
	unsigned char RE200BFlag = 0;
	unsigned char MQ2Flag = 0;
	unsigned char AH3020Flag = 0;
	
	switch(param)
	{
		//18b20�¶ȴ�����
		case Temperature18b20:
			{
				result = Get_Temperature();
				break;
			}

		//MIC����������
		case MIC:
			{
				
				SoundFlag = 0;
				for( i = 0;(i<100)&&(SoundFlag == 0);i++ )
				{
					for(j = 0;(j<1000)&&(SoundFlag == 0);j++)
					{
						if( PINB&0x02 )
						{
							k = 100;
							while(k--);
							if( PINB&0x02 )
							{
								SoundFlag = 1;
							}
						}
					}
				}
				result = SoundFlag;
				//1������0����
				break;
			}

		//������
		case TouchPad:
			{
				TouchFlag = 0;
				for( i = 0;(i<100)&&(TouchFlag == 0);i++ )
				{
					for(j = 0;(j<1000)&&(TouchFlag == 0);j++)
					{
						if( PINB&0x10 )
						{
							k = 100;
							while(k--);
							if( PINB&0x10 )
							{
								TouchFlag = 1;
							}
						}
					}
				}
				result = TouchFlag;
				//1�нӴ���0�޽Ӵ�
				break;
			}

		//�������裬���ص�ѹֵ
		case TemperatureResister:
			{
				result = StartADC(0);
				break;
			}

		//�������裬���ص�ѹֵ
		case LightResister:
			{
				result = StartADC(1);
				break;
			}

		//��ֵ�ѹ�����ص�ѹֵ
		case DifferentialVoltage:
			{
				result = StartADC(2);
				break;
			}

		//�������Դ����������ص�ѹֵ
		case AH3503:
			{
				result = StartADC(3);
				break;
			}

		//��е��������
		case MOVESW:
			{
				MoveFlag = 0;
				for( i = 0;(i<100)&&(MoveFlag == 0);i++ )
				{
					for(j = 0;(j<1000)&&(MoveFlag == 0);j++)
					{
						if( !(PINC&0x10) )
						{
							k = 100;
							while(k--);
							if( !(PINC&0x10) )
							{
								MoveFlag = 1;
							}
						}
					}
				}
				result = MoveFlag;
				//1�ж�����0�޶���
				break;
			}

		//�ɻɹܣ��͵�ƽ���ź�
		case REEDSW:
			{
				REEDFlag = 0;
				for( i = 0;(i<100)&&(REEDFlag == 0);i++ )
				{
					for(j = 0;(j<1000)&&(REEDFlag == 0);j++)
					{
						if( !(PINC&0x20) )
						{
							k = 100;
							while(k--);
							if( !(PINC&0x20) )
							{
								REEDFlag = 1;
							}
						}
					}
				}
				result = REEDFlag;
				//1�дű仯��0�޴ű仯
				break;
			}

		//���ͺ��⴫����
		case RE200B:
			{
				RE200BFlag = 0;
				for( i = 0;(i<100)&&(RE200BFlag == 0);i++ )
				{
					for(j = 0;(j<1000)&&(RE200BFlag == 0);j++)
					{
						if( PIND&0x04 )
						{
							k = 100;
							while(k--);
							if( PIND&0x04 )
							{
								RE200BFlag = 1;
							}
							
						}
					}
				}
				result = RE200BFlag;
				//1��0
				break;
			}

		//�������崫����
		case MQ2:
			{
				MQ2Flag = 0;
				for( i = 0;(i<100)&&(MQ2Flag == 0);i++ )
				{
					for(j = 0;(j<1000)&&(MQ2Flag == 0);j++)
					{
						if( PIND&0x08 )
						{
							k = 100;
							while(k--);
							if( PIND&0x08 )
							{
								MQ2Flag = 1;
							}
						}
					}
				}
				result = MQ2Flag;
				//1��0
				break;
			}

		//������俪�أ��򿪣�
		case StartInfraredRadio:
			{
				UCSRB = 0x98;
				
				StartCountHole();
				while(1)
				{
					DisplayData(HoleCountH*256+TCNT0);
					//���ӻ���SS����
					PORTB&=0xFB;
	
					//���ӻ���SS���ߣ������������
					PORTB|=0x04;
				}
				UCSRB = 0x18;
				break;
			}
			
		//������俪�أ��رգ�
		case FinishInfraredRadio:
			{
				FinishCountHole();
				result = HoleCountH*256+TCNT0;
				DisplayData(HoleCountH*256+TCNT0);
				//���ӻ���SS����
				PORTB&=0xFB;
				//���ӻ���SS���ߣ������������
				PORTB|=0x04;
				
				//TCCR0 = 0x00;
				break;
			}

		//ʪ�ȴ�����HS1101
		case HS1101:
			{
				result=GetHS1101();
				break;
			}

		//��������
		case AH3020:
			{
				AH3020Flag = 0;
				for( i = 0;(i<100)&&(AH3020Flag==0);i++ )
				{
					for(j = 0;j<1000;j++)
					{
						if( !(PIND&0x40) )
						{
							k = 100;
							while(k--);
							if( !(PIND&0x40) )
							{
								AH3020Flag = 1;
							}
						}
					}
				}
				result = AH3020Flag;
				//1��0
				break;
			}

		//��ѹ����
		case VoltageInput:
			{
				result = StartADC(6);
				break;
			}

		//��������
		case CurrentInput:
			{
				result = StartADC(7);
				break;
			}
		default:
			break;
	}
	DisplayData(result);
	
	//������ʾ
	if((result>=10)&&(result<100))
	{
		resultH = (unsigned char)(result/10);
		resultL = (unsigned char)(result-resultH*10);
		
		SendBuffer[0] = 0xBB;
		SendBuffer[1] = 0xFF;
		SendBuffer[2] = resultH;
		SendBuffer[3] = resultL;
		SendBuffer[4] = 0x00;
		SendBuffer[6] = 0xFF;
	}
	//��������ʾ
	else if((result>=0)&&(result<10))
	{
		resultH = (unsigned char)result;
		resultL = (unsigned char)(result*10)%10;
		SendBuffer[0] = 0xBB;
		SendBuffer[1] = 0xFF;
		SendBuffer[2] = resultH;
		SendBuffer[3] = resultL;
		SendBuffer[4] = 0x01;
		SendBuffer[6] = 0xFF;
	}
	else
	{
	 	SendBuffer[0] = 0xBB;
		SendBuffer[1] = 0xFF;
		SendBuffer[2] = 0xFF;
		SendBuffer[3] = 0xFF;
		SendBuffer[4] = 0x00;
		SendBuffer[6] = 0xFF;
	}
	UARTSendAMessage(SendBuffer,7);
}

