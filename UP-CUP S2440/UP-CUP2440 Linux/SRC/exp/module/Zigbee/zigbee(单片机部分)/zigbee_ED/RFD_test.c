#include "zigbee.h"		//Zigbee定义
#include "zAPL.h"	//Zigbee应用层

#include "SensorProcess.h"



//开关与LED
#define S2              3
#define S3              4
#define D1              0
#define D2              1
#define D3              2
//add by lyj
//方便调试，电灯程序
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
//zigbee网络收发数据缓冲区
static volatile unsigned char receive_package[125];
static volatile unsigned char send_package[125];
unsigned int send_package_length = 0;


//设置LED2
#define ZIGBEE_RFD_SET_LED 0xA0

//设置LED3
#define ZIGBEE_RFD_SET_SWITCH 0xA1

//设置SensorLED1
#define ZIGBEE_RFD_SET_SensorLED1 0xA2

//设置SensorLED2
#define ZIGBEE_RFD_SET_SensorLED2 0xA3

//设置SensorLED3
#define ZIGBEE_RFD_SET_SensorLED3 0xA4

//设置SensorLED4
#define ZIGBEE_RFD_SET_SensorLED4 0xA5

//读光敏电阻
#define ZIGBEE_RFD_READ_LightResistance 0xA6 

//读热敏电阻
#define ZIGBEE_RFD_READ_TemperatureResistance 0xA7

//查询麦克是否收到声音
#define ZIGBEE_RFD_READ_MIC 0xA8

//读湿度
#define ZIGBEE_RFD_READ_HumiditySensor 0xA9

//读电位器
#define ZIGBEE_RFD_READ_Potantiometers 0xAA

//读18b20温度传感器
#define ZIGBEE_RFD_READ_Temperature18b20 0xAB

//读编码开关的状态
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
//回调函数，由用户实现
//MAC层发完FRAME时触发
void AppMACFrameTransmitted(void);
//MAC层接到FRAME时触发
void AppMACFrameReceived(void);
//FRAME应答超时时触发
void AppMACFrameTimeOutOccurred(void);
//RFD加入网络时触发
BOOL AppOkayToAssociate(void);
//选择信道时触发
BOOL AppOkayToUseChannel(BYTE channel);

//硬件初始化函数
static void InitializeBoard(void);
BOOL ZIGBEE_RUN_ENDPOINT = FALSE;

int main(void)
{
	unsigned int k=0,j=0;

	//端点句柄
	EP_HANDLE test_ed,ep1,ep2,ep3;//BYTE
	//传送ID
	TRANS_ID trans_id = 0;//BYTE

	//协调器地址
	SHORT_ADDR coord;

	//地址配置开关
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
	//初始化硬件
	InitializeBoard();
	//传感器相关硬件初始化
	SensorPort_Init();
	GetSensorInformation(Temperature18b20);
	//第一次读18b20的数据为无效数据，上电先读一次

	//for test
	/*
	while(1)
	{
		Command_process(ZIGBEE_RFD_READ_TemperatureResistance,0x00);
	}
	*/

	//禁止看门狗
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
	//按键S2是否按下
	if ( (PINA & BM(S3)) == 0 )
	{
		//进入地址配置
		bInConfigMode = TRUE;
	}
*/

	/*从EEPROM里读取数据,判断地址是否分配*/
    if ( !APLIsAddressAssigned() )
    {	
		//进入地址配置
		ConsolePutString("MAC Address is not assigned, entering Configuration mode...\r\n");
		//通过串口发送一个字符串:介质访问控制层的地址没有分配。。。

		bInConfigMode = TRUE;
		//处于配置模式
	}else
		bInConfigMode = FALSE;

	//用串口配置地址   
	if (bInConfigMode)//MAC地址未定义
	{
		LONG_ADDR longAddr;//BYTE v[8]
		SHORT_ADDR short_addr;
		char idString[6];
		WORD_VAL idValue;  

		//SET_SensorLED1();//~~~~
		
		ConsolePutString("enter the board id(0-65535):\n");
		//点亮LED
		PORTA &= ~BM(D1);
		PORTA &= ~BM(D2);

		//SET_SensorLED2();//~~~~
/**		
		//等待释放按键S2
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
		//从串口调试助手输入6个字符，例如 “00000\”表示地址0, “00001\”表示地址1
        if ( ConsoleGetString(idString, sizeof(idString)) )//接收到的字符数大于0
        {
            idValue.Val = (WORD)atoi(idString);
			//返回idString的整数部分

		    longAddr.v[1] = idValue.byte.MSB;
            longAddr.v[0] = idValue.byte.LSB;
			//此前idValue.byte.MSB, idValue.byte.LSB  并没有初始化
			
			short_addr.byte.LSB = idValue.byte.LSB;
			short_addr.byte.MSB = idValue.byte.MSB;

			//设置长地址,向EEPROM中写数据，并复制给&macInfo.longAddr
			//longAddr.v[0], longAddr.v[1] 没有初始化
            APLPutLongAddress(&longAddr);

			//设置短地址,short_addr.Val, short_addr.v[] 没有初始化
			APLPutShortAddress(&short_addr);
			//向EEPROM中写数据，更新short_addr.byte.LSB = bb;short_addr.byte.MSB = bb;

			//更新CC2420地址
            APLUpdateAddressInfo();
        }

		//SET_SensorLED3();//~~~~
		
		//熄灭LED
		PORTA |= BM(D1);
		PORTA |= BM(D2);

		//SET_SensorLED4();//~~~~
		
		bInConfigMode = FALSE;
	}
	//地址配置结束

    //该函数初始化所有的协议栈模块。同时还初始化APL 状态机。
	APLInit();

	//该宏用于使能协议栈模块和RF 收发器。
	APLEnable();

    smApp = SM_APP_INIT;

    while( 1 )
    {
		//APLTask()  是协议栈任务，用来接收frame，必须保证调用这个任
		//务，否则不能接收数据。
		//APLTask  是个协同任务函数，按顺序调用每一个协议栈模块任务函数。
		//调用该函数使协议栈获取并处理进入的数据包。
		APLTask();

        switch(smApp)
        {
			case SM_APP_INIT:
				ConsolePutString("Attempting to join a coordinator...\r\n");
				//加入网络		
				APLJoin();
				//SET_SensorLED1();//~~~~
				while (1)
				{
					APLTask();
					//加入是否完毕，只有调用GetLastZError()返回ZCODE_NO_ERROR时，
					//才表明成功加入。
					//终端设备使用该宏来判断先前启动的连入过程是否已完成。
					if ( APLIsJoinComplete() )
					{
						if ( GetLastZError() == ZCODE_NO_ERROR )
						{
							ConsolePutString("Successfully associated.\r\n");
							 smApp = SM_APP_NORMAL_RUN;

							 
						}
						else
						{
							//加入失败，重新加入
							ConsolePutString("Failed to associate.\r\n");	
						}
						break;
					}
				}
				break;

			case SM_APP_NORMAL_RUN:

				//SET_SensorLED2();//~~~~ 
				//调试信息
				ConsolePutString("SM_APP_NORMAL_RUN:enter\r\n");
				ZIGBEE_RUN_ENDPOINT = TRUE;

				//设置协调器地址，网络的协调器地址都是0
				coord.Val = 0;
				//打开2号端点，test_ed为指向2号端点的句柄
				test_ed = APLOpenEP(2);
				ep3 = APLOpenEP(3);

				//设置alex_ed为当前端点
				//该函数把给定端点设置为活动端点。后面的函数调用
				//都将在该端点（即活动端点）上进行。
				APLSetEP(test_ed);

				//发送数据
				while (1)
				{
					//LED 1	工作指示灯
					SET_LED1();
					halWait(500);
					CLR_LED1_EN();

					APLTask();
					//先接收
					APLSetEP(ep3);

					//检查活动端点是否有等待接收的数据。
					if ( APLIsGetReady() )//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					{
						//SET_SensorLED2();//~~~~


						ConsolePutString("receive data..\n");
											SET_LED2();
					halWait(500);
					CLR_LED2_EN();

						//第一个字节无效
						//该函数从接收缓冲器中获取一个数据字节。
						APLGet();

						//数据包长度
						receive_length = APLGet();

						//demo中接收的数据是cmd +params
						for(i = 0;i < receive_length;i ++)
						{
							receive_package[i] = APLGet();
							//UART0_TransmitByte(receive_package[i]);
						}

						//SET_SensorLED3();//~~~~
						
						Command_process(receive_package[0],receive_package[1]);
						//命令、参数各一个字节

						//ConsolePutString("ClusterID:");//add by lyj for test
						//ConsolePutInitData(data[1],APLGetClusterID());

						//处理完一个数据包后必须丢弃，否则不能接受新的数据包
						APLDiscardRx();
						//将当前接收的帧从接收缓冲器中删除。

						Data_request = TRUE;
						count --;
					}

					//SET_SensorLED1();//~~~~
					
#if 1
					//后发送
					//发送数据给协调器
					//应用程序使用该函数来判断是否可以加载新帧。

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
						//配置不用更改
						currentMessageInfo.flags.Val = APS_MESSAGE_DIRECT |         \
										   APS_MESSAGE_SHORT_ADDRESS |  \
										   APS_MESSAGE_NO_ACK;          \
						//0x01

						currentMessageInfo.frameType              = MSG_FRAME;      
						currentMessageInfo.profileID              = MY_PROFILE_ID; //0x0000           
						currentMessageInfo.clusterID              = PICDEMZ_DEMO_CLUSTER_ID; //0           
						//源节点端点号
						currentMessageInfo.srcEP                  = 2;    
						//目的节点地址，设为协调器
						currentMessageInfo.destAddress.shortAddr  = coord;   
						//目的节点端点号，必须与目的节点端点号，否则接受不到数据
						currentMessageInfo.destEP                 = 2;  //modify by lyj

						/*
						//数据包长度
						currentMessageInfo.dataLength             = sizeof(receive_package);             
						//数据包内容
						currentMessageInfo.dataPointer            = receive_package;
						*/

						//数据包长度
						currentMessageInfo.dataLength			  = send_package_length;			 
						//数据包内容
						currentMessageInfo.dataPointer			  = send_package;

						
						currentMessageInfo.transactionID          = GENERATE_TRANS_ID;
						//发送数据
						trans_id = APLSendMessage( &currentMessageInfo );

						//SET_SensorLED2();//~~~~
						
						if (trans_id == TRANS_ID_INVALID)
						{
							//发送失败
							ConsolePutString("can not send app data\n");

							//SET_SensorLED3();//~~~~
							
						}
						//halWait(500);
						Data_request = FALSE;
						count = 10;
						//可以加入延时
					}
					//SET_SensorLED4();//~~~~
#endif
				}
				break;
			}
	}
	return 1;
}

//回调函数的空实现，用户可以加入代码
/*  此回调函数通知应用程序数据帧已发送。应用程序可使用此通知
点亮LED 或其他可视的指示器。*/
void AppMACFrameTransmitted(void)
{
}

/*  此回调函数通知应用程序接收到了新的有效数据帧。这仅仅是一
个通知（有可能已经处理了或未处理实际的帧）。 应用程序可使用
此通知点亮LED 或其他可视的指示器。*/
void AppMACFrameReceived(void)
{
}

/*  此回调函数通知应用程序远程节点未在MAC_ACK_WAIT_DURATION 内发送应
答。应用程序可使用此通知点亮LED 或其他可视的指示器。*/
void AppMACFrameTimeOutOccurred(void)
{
}

/*  这是一个主应用程序回调函数。当终端设备尝试连入可用网络时，
该协议栈会在其射频范围内找到一个协调器时调用此函数。在一个
射频范围内，不同的信道上可以有多个协调器；在这种情况下，请求
连入一个特定的协调器之前，协议栈会重复地查找协调器并调用这一
函数以通过应用程序的批准。应用程序可能会决定在选择要与之关联
的特定协调器之前搜集所有临近的协调器。*/
BOOL AppOkayToAssociate(void)
{
    return TRUE;
}

/*  此回调函数询问主应用程序是否可以使用给定的信道。*/
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
    //串口初始化
	ConsoleInit();
	//SPI初始化
	SPI_Init();
	//定时器1初始化
	TickInit();
/**
	//外部中断初始化
	EICRB |= BM(ISC71);
	EICRB &= ~BM(ISC70);//INT7为下降沿中断
	EIMSK |= BM(INT7);
*/

	//外部中断初始化
	EICRB |= BM(ISC71);
	EICRB &= ~BM(ISC70);//INT7为下降沿中断
	EIMSK |= BM(INT7);

	//开启全局中断
	sei();
/**
  	DDRA |= BM(D1) | BM(D2) | BM(D3);
	PORTA |= BM(D1) | BM(D2) | BM(D3);	
    //串口初始化
	ConsoleInit();
	//SPI初始化
	SPI_Init();
	//定时器1初始化
	TickInit();
	//外部中断初始化
	EICRB |= BM(ISC71);
	EICRB &= ~BM(ISC70);//INT7为下降沿中断
	EIMSK |= BM(INT7);
	//开启全局中断
	sei();
*/
}

//中断处理函数
//定时器1，更新系统节拍
SIGNAL(TIMER1_OVF_vect)
{
	TickUpdate();
}

//外部中断，frame发送完成中断
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
		//设置LED2
		case ZIGBEE_RFD_SET_LED:
			{
				if(param == 0x01)
					SET_LED2();
				else
					CLR_LED2_EN();
				break;
			}
		
		//设置LED3
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

		//设置SensorLED1
		case ZIGBEE_RFD_SET_SensorLED1:
			{
				if(param == 0x01)
					SET_SensorLED1();
				else
					CLR_SensorLED1_EN();
				break;
			}

		//设置SensorLED2
		case ZIGBEE_RFD_SET_SensorLED2:
			{
				if(param == 0x01)
					SET_SensorLED2();
				else
					CLR_SensorLED2_EN();
				break;
			}

		//设置SensorLED3
		case ZIGBEE_RFD_SET_SensorLED3:
			{
				if(param == 0x01)
					SET_SensorLED3();
				else
					CLR_SensorLED3_EN();
				break;
			}

		//设置SensorLED4
		case ZIGBEE_RFD_SET_SensorLED4:
			{
				if(param == 0x01)
					SET_SensorLED4();
				else
					CLR_SensorLED4_EN();
				break;
			}
		
		//读光敏电阻
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

		//读热敏电阻
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

		//查询麦克是否收到声音
		case ZIGBEE_RFD_READ_MIC:
			{
				result=GetSensorInformation(MIC);
				break;
			}

		//读湿度
		case ZIGBEE_RFD_READ_HumiditySensor:
			{
				result=GetSensorInformation(HumiditySensor);
				break;
			}

		//读电位器
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

		//读18b20温度传感器
		case ZIGBEE_RFD_READ_Temperature18b20:
			{
				result=GetSensorInformation(Temperature18b20);
				break;
			}

		//读编码开关的状态
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
	//每次发送一个字节，从高位到地位，一共两个字节

}






