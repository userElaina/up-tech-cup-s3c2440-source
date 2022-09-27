#include "zigbee.h"		//Zigbee����
#include "zAPL.h"	//ZigbeeӦ�ò�

#include "SensorProcess.h"



//������LED
#define S2              3
#define S3              4
#define D1              0
#define D2              1
#define D3              2
//add by lyj
//������ԣ���Ƴ���
//#ifndef LED_DEBUG
//#define LED_DEBUG
//#endif
//-------------------------------------------------------------------------------------------------------
// Port a
#define LED3_EN          0  // PA.0 - Output: VREG_EN to CC2420
#define LED2_EN          1  // PA.1 - Output: RESET_N to CC2420
#define LED1_EN               2  // PA.2 - Output: Green LED
#define FLASH_CS              3  // PA.3 - Output: Green LED
#define SERIAL_ID              4  // PA.4 - Output: Green LED
#define VREG_EN         5  // PA.5 - Output: VREG_EN to CC2420
#define RESET_N         6  // PA.6 - Output: RESET_N to CC2420
#define THERM           7  // PA.7 - Output: RESET_N to CC2420

//-------------------------------------------------------------------------------------------------------
// Port B
#define CSN             0  // PB.0 - Output: SPI Chip Select (CS_N)
#define SCK             1  // PB.1 - Output: SPI Serial Clock (SCLK)
#define MOSI            2  // PB.2 - Output: SPI Master out - slave in (MOSI)
#define MISO            3  // PB.3 - Input:  SPI Master in - slave out (MISO)
#define CCFIFO           7  // PB.7 - Output: GreenFIFO
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Port D
#define I2C_CLK               0 // PD.0 - Input:  FIFOP from CC2420
#define I2C_DATA            1 // PD.1 - Input:  FIFO from CC2420
#define UART1_RXD       2 // PD.2 - Input:  UART1 RXD
#define UART1_TXD       3 // PD.3 - Output: UART1 TXD
#define SFD             4 // PD.4 - Input:  SFD from CC2420
#define UART1_CLK      5 // PD.5 - Output: UART HW handshaking: RTS
#define CCA             6 // PD.6 - Input:  CCA from CC2420
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Port E
#define UART_RXD0     0 // PE.0 - Input : Joystick up button
#define UART_TXD0  1 // PE.1 - Input : Joystick right
#define AC_P 2 // PE.2 - Input : Joystick center
#define AC_S            3 // PE.3 - Output: Red LED
#define INT4            4 // PE.4 - Output: Yellow LED
#define INT5          5 // PE.5 - Input : Push button S2
#define FIFOP       6  // PE.6 - Input : FIFOP from CC2420
#define INT7          7 // PE.7 - Input : Push button S2......................??INT7
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Clears LED
#define CLR_LED3_EN()                  (PORTA |= BM(LED3_EN))
#define CLR_LED2_EN()                  (PORTA |= BM(LED2_EN))
#define CLR_LED1_EN()                  (PORTA |= BM(LED1_EN))


// Set LED
#define SET_LED3()                  (PORTA &= ~BM(LED3_EN))
#define SET_LED2()                  (PORTA &= ~BM(LED2_EN))
#define SET_LED1()                  (PORTA &= ~BM(LED1_EN))
//-------------------------------------------------------------------------------------------------------
//	void halWait(UINT16 timeout)
//
//	DESCRIPTION:
//		Runs an idle loop for [timeout] microseconds.
//
//  ARGUMENTS:
//      UINT16 timeout
//          The timeout in microseconds
//-------------------------------------------------------------------------------------------------------
// Useful stuff
#define NOP() asm volatile ("nop\n\t" ::)
//zigbee�����շ����ݻ�����
static volatile unsigned char receive_package[125];
static volatile unsigned char send_package[125];
unsigned int send_package_length = 0;


//����LED2
#define ZIGBEE_RFD_SET_LED 0xA0

//����LED3
#define ZIGBEE_RFD_SET_SWITCH 0xA1

//����SensorLED1
#define ZIGBEE_RFD_SET_SensorLED1 0xA2

//����SensorLED2
#define ZIGBEE_RFD_SET_SensorLED2 0xA3

//����SensorLED3
#define ZIGBEE_RFD_SET_SensorLED3 0xA4

//����SensorLED4
#define ZIGBEE_RFD_SET_SensorLED4 0xA5

//����������
#define ZIGBEE_RFD_READ_LightResistance 0xA6 

//����������
#define ZIGBEE_RFD_READ_TemperatureResistance 0xA7

//��ѯ����Ƿ��յ�����
#define ZIGBEE_RFD_READ_MIC 0xA8

//��ʪ��
#define ZIGBEE_RFD_READ_HumiditySensor 0xA9

//����λ��
#define ZIGBEE_RFD_READ_Potantiometers 0xAA

//��18b20�¶ȴ�����
#define ZIGBEE_RFD_READ_Temperature18b20 0xAB

//�����뿪�ص�״̬
#define ZIGBEE_RFD_READ_CODE 0xAC



void halWait(unsigned short timeout)
{
    // This sequence uses exactly 8 clock cycle for each round
    do
	{
        NOP();
        NOP();
        NOP();
        NOP();
    }
	while(--timeout);
} // halWait

//add end
//�ص����������û�ʵ��
//MAC�㷢��FRAMEʱ����
void AppMACFrameTransmitted(void);
//MAC��ӵ�FRAMEʱ����
void AppMACFrameReceived(void);
//FRAMEӦ��ʱʱ����
void AppMACFrameTimeOutOccurred(void);
//RFD��������ʱ����
BOOL AppOkayToAssociate(void);
//ѡ���ŵ�ʱ����
BOOL AppOkayToUseChannel(BYTE channel);

//Ӳ����ʼ������
static void InitializeBoard(void);
BOOL ZIGBEE_RUN_ENDPOINT = FALSE;

int main(void)
{
	unsigned int k=0,j=0;

	//�˵���
	EP_HANDLE test_ed,ep1,ep2,ep3;//BYTE
	//����ID
	TRANS_ID trans_id = 0;//BYTE

	//Э������ַ
	SHORT_ADDR coord;

	//��ַ���ÿ���
	BOOL bInConfigMode = FALSE;

	BOOL Data_request = FALSE;
	BYTE receive_length;
	BYTE send_data[2];
	int count = 10,i;
	BYTE command = 0xff;
	BYTE data[2] = {0,0};
    enum
    {
        SM_APP_INIT = 0,
        SM_APP_CONFIG_START,
        SM_APP_NORMAL_START,
        SM_APP_NORMAL_START_WAIT,
        SM_APP_NORMAL_RUN,
        SM_APP_SLEEP
    } smApp;
	//��ʼ��Ӳ��
	InitializeBoard();
	//���������Ӳ����ʼ��
	SensorPort_Init();
	GetSensorInformation(Temperature18b20);
	//��һ�ζ�18b20������Ϊ��Ч���ݣ��ϵ��ȶ�һ��

	//for test
	/*
	while(1)
	{
		Command_process(ZIGBEE_RFD_READ_TemperatureResistance,0x00);
	}
	*/

	//��ֹ���Ź�
	wdt_disable();

#ifdef LED_DEBUG	
		//LED 1	    
		SET_LED1();
		halWait(50000);
		CLR_LED1_EN();
		halWait(50000);
		//LED 2	    
		SET_LED2();
		halWait(50000);
		CLR_LED2_EN();
		halWait(50000);
		//LED 3
		SET_LED3();
		halWait(50000);
		CLR_LED3_EN();
		halWait(50000);	
#endif
/**
	//����S2�Ƿ���
	if ( (PINA & BM(S3)) == 0 )
	{
		//�����ַ����
		bInConfigMode = TRUE;
	}
*/

	/*��EEPROM���ȡ����,�жϵ�ַ�Ƿ����*/
    if ( !APLIsAddressAssigned() )
    {	
		//�����ַ����
		ConsolePutString("MAC Address is not assigned, entering Configuration mode...\r\n");
		//ͨ�����ڷ���һ���ַ���:���ʷ��ʿ��Ʋ�ĵ�ַû�з��䡣����

		bInConfigMode = TRUE;
		//��������ģʽ
	}else
		bInConfigMode = FALSE;

	//�ô������õ�ַ   
	if (bInConfigMode)//MAC��ַδ����
	{
		LONG_ADDR longAddr;//BYTE v[8]
		SHORT_ADDR short_addr;
		char idString[6];
		WORD_VAL idValue;  

		//SET_SensorLED1();//~~~~
		
		ConsolePutString("enter the board id(0-65535):\n");
		//����LED
		PORTA &= ~BM(D1);
		PORTA &= ~BM(D2);

		//SET_SensorLED2();//~~~~
/**		
		//�ȴ��ͷŰ���S2
		while ((PINA&BM(S3)) == 0)
		{
		}
*/
		longAddr.v[7] = MAC_LONG_ADDR_BYTE7;//0x00
		longAddr.v[6] = MAC_LONG_ADDR_BYTE6;//0x04
		longAddr.v[5] = MAC_LONG_ADDR_BYTE5;//0xa3
		longAddr.v[4] = 0x00;
		longAddr.v[3] = 0x00;
		longAddr.v[2] = 0x00;
		//�Ӵ��ڵ�����������6���ַ������� ��00000\����ʾ��ַ0, ��00001\����ʾ��ַ1
        if ( ConsoleGetString(idString, sizeof(idString)) )//���յ����ַ�������0
        {
            idValue.Val = (WORD)atoi(idString);
			//����idString����������

		    longAddr.v[1] = idValue.byte.MSB;
            longAddr.v[0] = idValue.byte.LSB;
			//��ǰidValue.byte.MSB, idValue.byte.LSB  ��û�г�ʼ��
			
			short_addr.byte.LSB = idValue.byte.LSB;
			short_addr.byte.MSB = idValue.byte.MSB;

			//���ó���ַ,��EEPROM��д���ݣ������Ƹ�&macInfo.longAddr
			//longAddr.v[0], longAddr.v[1] û�г�ʼ��
            APLPutLongAddress(&longAddr);

			//���ö̵�ַ,short_addr.Val, short_addr.v[] û�г�ʼ��
			APLPutShortAddress(&short_addr);
			//��EEPROM��д���ݣ�����short_addr.byte.LSB = bb;short_addr.byte.MSB = bb;

			//����CC2420��ַ
            APLUpdateAddressInfo();
        }

		//SET_SensorLED3();//~~~~
		
		//Ϩ��LED
		PORTA |= BM(D1);
		PORTA |= BM(D2);

		//SET_SensorLED4();//~~~~
		
		bInConfigMode = FALSE;
	}
	//��ַ���ý���

    //�ú�����ʼ�����е�Э��ջģ�顣ͬʱ����ʼ��APL ״̬����
	APLInit();

	//�ú�����ʹ��Э��ջģ���RF �շ�����
	APLEnable();

    smApp = SM_APP_INIT;

    while( 1 )
    {
		//APLTask()  ��Э��ջ������������frame�����뱣֤���������
		//�񣬷����ܽ������ݡ�
		//APLTask  �Ǹ�Эͬ����������˳�����ÿһ��Э��ջģ����������
		//���øú���ʹЭ��ջ��ȡ�������������ݰ���
		APLTask();

        switch(smApp)
        {
			case SM_APP_INIT:
				ConsolePutString("Attempting to join a coordinator...\r\n");
				//��������		
				APLJoin();
				//SET_SensorLED1();//~~~~
				while (1)
				{
					APLTask();
					//�����Ƿ���ϣ�ֻ�е���GetLastZError()����ZCODE_NO_ERRORʱ��
					//�ű����ɹ����롣
					//�ն��豸ʹ�øú����ж���ǰ��������������Ƿ�����ɡ�
					if ( APLIsJoinComplete() )
					{
						if ( GetLastZError() == ZCODE_NO_ERROR )
						{
							ConsolePutString("Successfully associated.\r\n");
							 smApp = SM_APP_NORMAL_RUN;

							 
						}
						else
						{
							//����ʧ�ܣ����¼���
							ConsolePutString("Failed to associate.\r\n");	
						}
						break;
					}
				}
				break;

			case SM_APP_NORMAL_RUN:

				//SET_SensorLED2();//~~~~ 
				//������Ϣ
				ConsolePutString("SM_APP_NORMAL_RUN:enter\r\n");
				ZIGBEE_RUN_ENDPOINT = TRUE;

				//����Э������ַ�������Э������ַ����0
				coord.Val = 0;
				//��2�Ŷ˵㣬test_edΪָ��2�Ŷ˵�ľ��
				test_ed = APLOpenEP(2);
				ep3 = APLOpenEP(3);

				//����alex_edΪ��ǰ�˵�
				//�ú����Ѹ����˵�����Ϊ��˵㡣����ĺ�������
				//�����ڸö˵㣨����˵㣩�Ͻ��С�
				APLSetEP(test_ed);

				//��������
				while (1)
				{
					//LED 1	����ָʾ��
					SET_LED1();
					halWait(500);
					CLR_LED1_EN();

					APLTask();
					//�Ƚ���
					APLSetEP(ep3);

					//����˵��Ƿ��еȴ����յ����ݡ�
					if ( APLIsGetReady() )//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					{
						//SET_SensorLED2();//~~~~


						ConsolePutString("receive data..\n");
											SET_LED2();
					halWait(500);
					CLR_LED2_EN();

						//��һ���ֽ���Ч
						//�ú����ӽ��ջ������л�ȡһ�������ֽڡ�
						APLGet();

						//���ݰ�����
						receive_length = APLGet();

						//demo�н��յ�������cmd +params
						for(i = 0;i < receive_length;i ++)
						{
							receive_package[i] = APLGet();
							//UART0_TransmitByte(receive_package[i]);
						}

						//SET_SensorLED3();//~~~~
						
						Command_process(receive_package[0],receive_package[1]);
						//���������һ���ֽ�

						//ConsolePutString("ClusterID:");//add by lyj for test
						//ConsolePutInitData(data[1],APLGetClusterID());

						//������һ�����ݰ�����붪���������ܽ����µ����ݰ�
						APLDiscardRx();
						//����ǰ���յ�֡�ӽ��ջ�������ɾ����

						Data_request = TRUE;
						count --;
					}

					//SET_SensorLED1();//~~~~
					
#if 1
					//����
					//�������ݸ�Э����
					//Ӧ�ó���ʹ�øú������ж��Ƿ���Լ�����֡��

					//for test
					//if (APLIsPutReady())SET_SensorLED2();//~~~~
					//if(Data_request)SET_SensorLED3();//~~~~



					if (APLIsPutReady() && Data_request)
					{
						//SET_SensorLED1();//~~~~
						
						ConsolePutString("try send !\n");
						send_data[0] = receive_package[0];
						send_data[1] = receive_package[1];
						APLSetEP(test_ed);
						//���ò��ø���
						currentMessageInfo.flags.Val = APS_MESSAGE_DIRECT |         \
										   APS_MESSAGE_SHORT_ADDRESS |  \
										   APS_MESSAGE_NO_ACK;          \
						//0x01

						currentMessageInfo.frameType              = MSG_FRAME;      
						currentMessageInfo.profileID              = MY_PROFILE_ID; //0x0000           
						currentMessageInfo.clusterID              = PICDEMZ_DEMO_CLUSTER_ID; //0           
						//Դ�ڵ�˵��
						currentMessageInfo.srcEP                  = 2;    
						//Ŀ�Ľڵ��ַ����ΪЭ����
						currentMessageInfo.destAddress.shortAddr  = coord;   
						//Ŀ�Ľڵ�˵�ţ�������Ŀ�Ľڵ�˵�ţ�������ܲ�������
						currentMessageInfo.destEP                 = 2;  //modify by lyj

						/*
						//���ݰ�����
						currentMessageInfo.dataLength             = sizeof(receive_package);             
						//���ݰ�����
						currentMessageInfo.dataPointer            = receive_package;
						*/

						//���ݰ�����
						currentMessageInfo.dataLength			  = send_package_length;			 
						//���ݰ�����
						currentMessageInfo.dataPointer			  = send_package;

						
						currentMessageInfo.transactionID          = GENERATE_TRANS_ID;
						//��������
						trans_id = APLSendMessage( &currentMessageInfo );

						//SET_SensorLED2();//~~~~
						
						if (trans_id == TRANS_ID_INVALID)
						{
							//����ʧ��
							ConsolePutString("can not send app data\n");

							//SET_SensorLED3();//~~~~
							
						}
						//halWait(500);
						Data_request = FALSE;
						count = 10;
						//���Լ�����ʱ
					}
					//SET_SensorLED4();//~~~~
#endif
				}
				break;
			}
	}
	return 1;
}

//�ص������Ŀ�ʵ�֣��û����Լ������
/*  �˻ص�����֪ͨӦ�ó�������֡�ѷ��͡�Ӧ�ó����ʹ�ô�֪ͨ
����LED ���������ӵ�ָʾ����*/
void AppMACFrameTransmitted(void)
{
}

/*  �˻ص�����֪ͨӦ�ó�����յ����µ���Ч����֡���������һ
��֪ͨ���п����Ѿ������˻�δ����ʵ�ʵ�֡���� Ӧ�ó����ʹ��
��֪ͨ����LED ���������ӵ�ָʾ����*/
void AppMACFrameReceived(void)
{
}

/*  �˻ص�����֪ͨӦ�ó���Զ�̽ڵ�δ��MAC_ACK_WAIT_DURATION �ڷ���Ӧ
��Ӧ�ó����ʹ�ô�֪ͨ����LED ���������ӵ�ָʾ����*/
void AppMACFrameTimeOutOccurred(void)
{
}

/*  ����һ����Ӧ�ó���ص����������ն��豸���������������ʱ��
��Э��ջ��������Ƶ��Χ���ҵ�һ��Э����ʱ���ô˺�������һ��
��Ƶ��Χ�ڣ���ͬ���ŵ��Ͽ����ж��Э����������������£�����
����һ���ض���Э����֮ǰ��Э��ջ���ظ��ز���Э������������һ
������ͨ��Ӧ�ó������׼��Ӧ�ó�����ܻ������ѡ��Ҫ��֮����
���ض�Э����֮ǰ�Ѽ������ٽ���Э������*/
BOOL AppOkayToAssociate(void)
{
    return TRUE;
}

/*  �˻ص�����ѯ����Ӧ�ó����Ƿ����ʹ�ø������ŵ���*/
BOOL AppOkayToUseChannel(BYTE channel)
{
     return TRUE;
}
//add by lyj 
#define PORT_INIT() \
    do { \
        SFIOR |= BM(PUD); \
        DDRA  =  BM(RESET_N) | BM(VREG_EN) |BM(LED3_EN) | BM(LED2_EN)| BM(LED1_EN); \
        PORTA = BM(LED3_EN)|BM(LED2_EN)|BM(LED1_EN)|BM(RESET_N); \
        DDRB  = BM(MOSI) | BM(SCK) | BM(CSN) | BM(CCFIFO) ; \
        PORTB =  BM(SCK) | BM(CSN); \
        DDRD &= ~BM(4);\
    } while (0)

//add end
static void InitializeBoard(void)
{
//  	DDRA |= BM(RESET_N) | BM(LED3_EN) | BM(LED2_EN)| BM(LED1_EN);
//	PORTA |= BM(LED3_EN)|BM(LED2_EN)|BM(LED1_EN)|BM(RESET_N);	
	PORT_INIT();
    //���ڳ�ʼ��
	ConsoleInit();
	//SPI��ʼ��
	SPI_Init();
	//��ʱ��1��ʼ��
	TickInit();
/**
	//�ⲿ�жϳ�ʼ��
	EICRB |= BM(ISC71);
	EICRB &= ~BM(ISC70);//INT7Ϊ�½����ж�
	EIMSK |= BM(INT7);
*/

	//�ⲿ�жϳ�ʼ��
	EICRB |= BM(ISC71);
	EICRB &= ~BM(ISC70);//INT7Ϊ�½����ж�
	EIMSK |= BM(INT7);

	//����ȫ���ж�
	sei();
/**
  	DDRA |= BM(D1) | BM(D2) | BM(D3);
	PORTA |= BM(D1) | BM(D2) | BM(D3);	
    //���ڳ�ʼ��
	ConsoleInit();
	//SPI��ʼ��
	SPI_Init();
	//��ʱ��1��ʼ��
	TickInit();
	//�ⲿ�жϳ�ʼ��
	EICRB |= BM(ISC71);
	EICRB &= ~BM(ISC70);//INT7Ϊ�½����ж�
	EIMSK |= BM(INT7);
	//����ȫ���ж�
	sei();
*/
}

//�жϴ�����
//��ʱ��1������ϵͳ����
SIGNAL(TIMER1_OVF_vect)
{
	TickUpdate();
}

//�ⲿ�жϣ�frame��������ж�
SIGNAL(INT7_vect)
{
#ifdef LED_DEBUG
		//LED 2	    
		SET_LED2();
		halWait(25000);
		CLR_LED2_EN();
		halWait(25000);
#endif
	APL_ISR();

}

void Command_process(BYTE cmd,BYTE param)
{
	unsigned int result = 0;
	unsigned char resultH,resultL;
	unsigned int i = 0;
	switch(cmd)
	{
		//����LED2
		case ZIGBEE_RFD_SET_LED:
			{
				if(param == 0x01)
					SET_LED2();
				else
					CLR_LED2_EN();
				break;
			}
		
		//����LED3
		case ZIGBEE_RFD_SET_SWITCH:
			{

				//SET_SensorLED1();//~~~~

				if(param == 0x01)
				{
					SET_LED3();
				}
				else
				{
					CLR_LED3_EN();
					//SET_SensorLED2();//~~~~
				}
				break;
			}

		//����SensorLED1
		case ZIGBEE_RFD_SET_SensorLED1:
			{
				if(param == 0x01)
					SET_SensorLED1();
				else
					CLR_SensorLED1_EN();
				break;
			}

		//����SensorLED2
		case ZIGBEE_RFD_SET_SensorLED2:
			{
				if(param == 0x01)
					SET_SensorLED2();
				else
					CLR_SensorLED2_EN();
				break;
			}

		//����SensorLED3
		case ZIGBEE_RFD_SET_SensorLED3:
			{
				if(param == 0x01)
					SET_SensorLED3();
				else
					CLR_SensorLED3_EN();
				break;
			}

		//����SensorLED4
		case ZIGBEE_RFD_SET_SensorLED4:
			{
				if(param == 0x01)
					SET_SensorLED4();
				else
					CLR_SensorLED4_EN();
				break;
			}
		
		//����������
		case ZIGBEE_RFD_READ_LightResistance:
			{
				result=GetSensorInformation(LightResistance);
				//for test
				/*
				if( result == 0 )SET_SensorLED1();
				else if( result == 1 )SET_SensorLED2();
				else if( result == 2 )SET_SensorLED3();
				else;
				i=5000;
				while(i--);
				CLR_SensorLED4_EN();
			 	CLR_SensorLED3_EN();
				CLR_SensorLED2_EN();
				CLR_SensorLED1_EN();
				*/
				break;
			}

		//����������
		case ZIGBEE_RFD_READ_TemperatureResistance:
			{
				result=GetSensorInformation(TemperatureResistance);
				//for test
				/*
				if( result == 0 )SET_SensorLED1();
				else if( result == 1 )SET_SensorLED2();
				else if( result == 2 )SET_SensorLED3();
				else;
				i=5000;
				while(i--);
				CLR_SensorLED4_EN();
			 	CLR_SensorLED3_EN();
				CLR_SensorLED2_EN();
				CLR_SensorLED1_EN();
				*/
				break;
			}

		//��ѯ����Ƿ��յ�����
		case ZIGBEE_RFD_READ_MIC:
			{
				result=GetSensorInformation(MIC);
				break;
			}

		//��ʪ��
		case ZIGBEE_RFD_READ_HumiditySensor:
			{
				result=GetSensorInformation(HumiditySensor);
				break;
			}

		//����λ��
		case ZIGBEE_RFD_READ_Potantiometers:
			{
				// by sprife
						SET_LED2();
		halWait(50000);
		CLR_LED2_EN();
		halWait(50000);
				result=GetSensorInformation(Potantiometers);
				break;
			}

		//��18b20�¶ȴ�����
		case ZIGBEE_RFD_READ_Temperature18b20:
			{
				result=GetSensorInformation(Temperature18b20);
				break;
			}

		//�����뿪�ص�״̬
		case ZIGBEE_RFD_READ_CODE:
			{//by sprife
					SET_LED2();
		halWait(50000);
		CLR_LED2_EN();
		halWait(50000);
				result = PINC&0x0F;
				break;
			}
		
		default:
			break;

	}
	resultH = result/256;
	resultL = result-256*resultH;
	send_package_length = 2;
	send_package[0] = resultH;
	send_package[1] = resultL;
	//ÿ�η���һ���ֽڣ��Ӹ�λ����λ��һ�������ֽ�

}






