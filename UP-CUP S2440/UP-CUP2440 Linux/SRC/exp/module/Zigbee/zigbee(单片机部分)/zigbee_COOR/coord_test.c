#include "zigbee.h"		// Zigbee定义
#include "zAPL.h"	//Zigbee应用层	

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
//传感器编号
#define LightResistance 		0
#define TemperatureResistance 	1
#define SoundSensor 			2
#define HumiditySensor 			3
#define Potantiometers 			4
#define Temperature18b20 		5

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
#define INT5          7 // PE.7 - Input : Push button S2
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
void halWait(unsigned short timeout) {

    // This sequence uses exactly 8 clock cycle for each round
    do {
        NOP();
        NOP();
        NOP();
        NOP();
    } while (--timeout);

} // halWait

//add end

//回调函数
//MAC层发完FRAME时触发
void AppMACFrameTransmitted(void);
//MAC层接到FRAME时触发
void AppMACFrameReceived(void);
//FRAME应答超时时触发
void AppMACFrameTimeOutOccurred(void);
//选择信道时触发
BOOL AppOkayToUseChannel(BYTE channel);
//当有RFD请求加入时触发
BOOL AppOkayToAcceptThisNode(LONG_ADDR *longAddr);
//当RFD加入后触发
void AppNewNodeJoined(LONG_ADDR *nodeAddr, BOOL bIsRejoined);
//当RFD离开网络触发
void AppNodeLeft(LONG_ADDR *nodeAddr);

//硬件初始函数
static void InitializeBoard(void);
static unsigned char UART0_TxBuf[128]={0xBB,0xFF,0x06,0x00,0xA2,0x00,0x00,0x00,0x00,0xFF};
//从串口接收有效数据缓冲区
static volatile unsigned char receive_package[125];
static volatile unsigned char send_package[125];
static volatile SHORT_ADDR current_rev_dest_addr;
static volatile SHORT_ADDR current_rev_src_addr;
//将收到的数据全部传回，在应用程序端解析
static volatile BYTE current_rev_data_len;//传回的数据长度是已知的
static volatile BYTE current_rev_srcEP;
static volatile BYTE current_rev_destEP;
static volatile BYTE current_send_dest_addr;
static volatile BYTE current_send_src_addr;
static volatile BYTE current_send_data_len;//发送的数据只有一个字节
static volatile BYTE current_send_cmd;//全部使用的是cmd
static volatile BYTE current_send_params;
//
//串口命令
#define ZIGBEE_CMD_SEND 0x00
#define ZIGBEE_CMD_REV  0x01
#define ZIGBEE_CMD_SET_ADDR 0x10


//flag 标志位
//是否收到数据
static volatile BOOL current_rev_flag = FALSE;
static int i;


int main(void)
{
    //地址配置开关
	BOOL bInConfigMode = FALSE;
	
	BYTE receive_length;

	BYTE data[2] = {0,0};
	
	EP_HANDLE EP[NUM_DEFINED_ENDPOINTS],current_EP;

	BYTE dest_addr,src_addr;

	//协调器地址
	SHORT_ADDR Rfd_dest;

	//传送ID
	TRANS_ID trans_id = 0;

	//互斥锁
	BOOL send_lock = FALSE;
    enum
    {
        SM_APP_INIT = 0,
        SM_APP_INIT_RUN,
        SM_APP_NORMAL_START,
        SM_APP_NORMAL_RUN
    } smApp;
	
	//初始化硬件
	InitializeBoard();
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

//第一次使用该模块时需要需要，先配置地址，默认的地址是0x10;
//读取EEPROM中的数据，判断是否已经配置了地址
    if (!APLIsAddressAssigned())
    {
    	LONG_ADDR longAddr;
		SHORT_ADDR short_addr;
		longAddr.v[7] = MAC_LONG_ADDR_BYTE7;
		longAddr.v[6] = MAC_LONG_ADDR_BYTE6;
		longAddr.v[5] = MAC_LONG_ADDR_BYTE5;
		longAddr.v[4] = 0x00;
		longAddr.v[3] = 0x00;
		longAddr.v[2] = 0x00;
		longAddr.v[1] = 0x00;//
        longAddr.v[0] = 0x0a;//10
		short_addr.byte.LSB = 0x0a;//
		short_addr.byte.MSB = 0x00;//10
		//设置长地址
        APLPutLongAddress(&longAddr);
		//设置短地址
		APLPutShortAddress(&short_addr);
		//更新CC2420地址
        APLUpdateAddressInfo();
    }

	//初始化协议栈
    APLInit();
	//使能协议栈 
    APLEnable();
	//建立网络  
    APLNetworkInit();

    smApp = SM_APP_INIT;
	
	
    while (1)
    {
//    	ConsolePutString(__FUNCTION__);
//		ConsolePutString("\r\n");
        //APLTask()是协议栈任务，用来接收frame，必须保证调用这个任务，否则不能接收数据
        APLTask();

        switch (smApp)
        {
			case SM_APP_INIT:
				ConsolePutString("Starting a new network...\r\n");
				smApp = SM_APP_INIT_RUN;

				//SET_LED3();//~~~~
				
				break;

			case SM_APP_INIT_RUN:
				//网络是否建立完毕,只有调用GetLastZError()返回ZCODE_NO_ERROR时，才表明成功建立
				if ( APLIsNetworkInitComplete() )
				{
					if ( GetLastZError() == ZCODE_NO_ERROR )
					{
						ConsolePutString("New network successfully started.\r\n");
						//建立网络
						APLNetworkForm();
						smApp = SM_APP_NORMAL_START;
					}
					else
					{
						//网络建立失败，重新建立
						ConsolePutString("There was an error starting network.\r\n");
						smApp = SM_APP_INIT;
					}
				}
				break;

			case SM_APP_NORMAL_START:
				//允许RFD加入网络
				//SET_LED3();//~~~~
				APLPermitAssociation();
				ConsolePutString("SM_APP_NORMAL_START: permit join.\r\n");
				smApp = SM_APP_NORMAL_RUN;

				//SET_LED2();//~~~~
				
				break;

			case SM_APP_NORMAL_RUN:
				//打开2号端点，ep为指向2号端点的句柄
				for(i = 0;i <NUM_DEFINED_ENDPOINTS;i ++)         //0~4
					EP[i] = APLOpenEP(i);
				//默认设置2号端点为当前端点
				current_EP = EP[2];
				//应用主循环
				while(1){
					//SET_LED3();//~~~~
					APLSetEP(current_EP);
					APLTask();
					//SET_LED3();//~~~~
#if 1
					//是否接收到数据
					if ( APLIsGetReady() )//~~~~~~~~~~~~~~~~~~~~~~~~~~~
					{

						//SET_LED3();//~~~~

						ConsolePutString("receive data..\n");
						//第一个字节无效
						APLGet();
						//数据包长度
						receive_length = APLGet();
						current_rev_data_len = receive_length;
						//通过串口将接收数据打印出来
						for(i = 0;i < receive_length;i ++){
							UART0_TxBuf[8 + i] = APLGet();
							//直接存放在串口发送的数据缓冲中
							
							//for test
							//UART0_TransmitByte(UART0_TxBuf[8 + i]);
	
						}

						current_rev_dest_addr = APLGetDestShortAddr();
						current_rev_src_addr = APLGetSrcShortAddr();



						
					//处理完一个数据包后必须丢弃，否则不能接受新的数据包
						APLDiscardRx();
						current_rev_flag = TRUE;
					}
#endif
					//解析串口命令
					//receive_a_frame   在串口接收中断里被修改了
					if(if_receive_a_frame()){
						
						//SET_LED3();//~~~~

						decode_rec();
						if(if_command_valid()){
							
							//SET_LED3();//~~~~
							
							Reset_command_valid();
							switch(get_command()){
								case ZIGBEE_CMD_SEND:

									//SET_LED3();//~~~~
									
									Send_package(EP);break;
								case ZIGBEE_CMD_REV:
									Rev_package();break;
								case ZIGBEE_CMD_SET_ADDR:
									Set_address();break;
								default:break;
							}
						}else{
							test_for();
							//命令解析错误，返回命令
						}

						Reset_receive_frame();

					}
					if(current_rev_flag)
					{
						SET_LED2();

						//SET_LED3();//~~~~
						
					}
					else
						CLR_LED2_EN();
					//LED 1正常工作指示灯
					SET_LED1();
					halWait(500);
					CLR_LED1_EN();
				}

				break;
        }
    }
	return 1;
}






//回调函数空实现，用户可以加入代码
void AppMACFrameTransmitted(void)
{
}


void AppMACFrameReceived(void)
{
}

void AppMACFrameTimeOutOccurred(void)
{
}

BOOL AppOkayToUseChannel(BYTE channel)
{
 return TRUE;
}

BOOL AppOkayToAcceptThisNode(LONG_ADDR *longAddr)
{
	return TRUE;
}

void AppNewNodeJoined(LONG_ADDR *nodeAddr, BOOL bIsRejoined)
{
}

void AppNodeLeft(LONG_ADDR *nodeAddr)
{

}
//add by lyj 
//初始化的时候不必初始化cc2420的相关定义，系统在调用
//函数的时候会自动调用相关的逻辑
/**
#define PORT_INIT() \
    do { \
        SFIOR |= BM(PUD); \
        DDRA  =  BM(RESET_N) | BM(VREG_EN) |BM(LED3_EN) | BM(LED2_EN)| BM(LED1_EN); \
        PORTA = BM(LED3_EN)|BM(LED2_EN)|BM(LED1_EN)|BM(RESET_N); \
        DDRB  = BM(MOSI) | BM(SCK) | BM(CSN) | BM(CCFIFO) ; \
        PORTB =  BM(SCK) | BM(CSN);\
    } while (0)
*/
#define PORT_INIT() \
    do { \
        SFIOR |= BM(PUD); \
        DDRA  =  BM(RESET_N) | BM(VREG_EN) |BM(LED3_EN) | BM(LED2_EN)| BM(LED1_EN); \
        PORTA = BM(LED3_EN)|BM(LED2_EN)|BM(LED1_EN)|BM(RESET_N); \
        DDRB  = BM(MOSI) | BM(SCK) | BM(CSN) | BM(CCFIFO) ; \
        PORTB =  BM(SCK) | BM(CSN);\
        DDRD &= ~BM(4);\
    } while (0)

//add end
static void InitializeBoard(void)
{
	PORT_INIT();
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
/******************************************
*
*
* 工具函数将来作为单独的文件来处理
*
*
*******************************************/
//从串口调试助手输入6个字符，例如 “00000\”表示地址0, “00001\”表示地址1
//char idString[6];
// AA FF 06 10 00 0A 00 00 FF
//  0   1  2   3   4   5   6  7  9
//本地的地址是10
void cmd_set_address(){
	LONG_ADDR longAddr;
	SHORT_ADDR short_addr;
	WORD_VAL idValue;
		
	longAddr.v[7] = MAC_LONG_ADDR_BYTE7;
	longAddr.v[6] = MAC_LONG_ADDR_BYTE6;
	longAddr.v[5] = MAC_LONG_ADDR_BYTE5;
	longAddr.v[4] = 0x00;
	longAddr.v[3] = 0x00;
	longAddr.v[2] = 0x00;
           
//	idValue.Val = (WORD)atoi(idString);
	idValue.byte.MSB = receive_data[4];
	idValue.byte.LSB = receive_data[5];
    longAddr.v[1] = idValue.byte.MSB;
    longAddr.v[0] = idValue.byte.LSB;
	short_addr.byte.LSB = idValue.byte.LSB;
	short_addr.byte.MSB = idValue.byte.MSB;
	//设置长地址
    APLPutLongAddress(&longAddr);
	//设置短地址
	APLPutShortAddress(&short_addr);
	//更新CC2420地址
    APLUpdateAddressInfo();   
}




//根据用户的输入信息，获取设置发送数据帧
//需要将二进制转化为十进制
//命令帧格式
// AA FF 06 00 03 02 00 0C 34 35 FF
//包含的数据目的地址(XX XX)，目的端点(XX)，源端点(XX)，命令(XX)
//返回发送结果
//帧头   |   帧长度|命令|目标端点|源端点|目标地址|     命令| 参数 |结尾
// AA  FF           06               00              03                 02                MSB LSB                                       FF
// 0      1           2                 3               4                   5                  6    7                8            9           10
void cmd_set_send_info(){
		SHORT_ADDR Rfd_dest;
		BYTE *pointer = "ab";
		//注意在发送的时候也要使用联合体
		Rfd_dest.byte.MSB = receive_data[6];
		Rfd_dest.byte.LSB = receive_data[7];
		pointer[0] = receive_data[8];
		pointer[1] = receive_data[9];
		//配置不用更改
		currentMessageInfo.flags.Val = APS_MESSAGE_DIRECT |         \
										   APS_MESSAGE_SHORT_ADDRESS |  \
										   APS_MESSAGE_NO_ACK;          \
		currentMessageInfo.frameType              = MSG_FRAME;      
		currentMessageInfo.profileID              = MY_PROFILE_ID;            
		currentMessageInfo.clusterID              = PICDEMZ_DEMO_CLUSTER_ID;            
		//源节点端点号
		currentMessageInfo.srcEP                  = receive_data[5];    
		//目的节点地址，设为协调器
		currentMessageInfo.destAddress.shortAddr  = Rfd_dest;   
		//目的节点端点号,必须与目的节点端点号，否则接受不到数据
		currentMessageInfo.destEP                 = receive_data[4];  //modify by lyj
		//数据包长度
		currentMessageInfo.dataLength             = strlen(pointer);             
		//数据包内容
		currentMessageInfo.dataPointer            = pointer;

		currentMessageInfo.transactionID          = GENERATE_TRANS_ID;
}
// AA FF 06 01 XX XX XX XX FF
//获取当前帧数据
//返回帧格式
// BB FF XX 01 
//包含数据 : 发送的源地址(shortaddr)(XX XX)，源端点(XX)，目标端点(XX)，
//返回的数据(XX XX XX XX XX XX)
void cmd_set_rev_info(BYTE *data)
{
	
}

//0xBB,0xFF,0x06,0x00,0xA2,0x00,0x00,0x00,0x00,0xFF
//BB FF 07 00 00 00 00 00 00 FF
//0    1   2  3  4   5  6   7  8   9
void Send_package(EP_HANDLE *End_Point)
{
	//传送ID
	TRANS_ID trans_id = 0;
	APLSetEP(End_Point[receive_data[5]]);
	//乱代码
	
	cmd_set_send_info();
	//发送数据
	trans_id = APLSendMessage( &currentMessageInfo );

	//SET_LED3();//~~~~
	
	if (trans_id == TRANS_ID_INVALID)
	{

		//SET_LED3();//~~~~

		UART0_TxBuf[2]=0x07;//数据长度
		UART0_TxBuf[3]=ZIGBEE_CMD_SEND;
		UART0_TxBuf[4]=0;//发送结果
		UART0_TxBuf[5]=0;
		UART0_TxBuf[6]=0;
		UART0_TxBuf[7]=0;
		UART0_TxBuf[8]=0;
		UART0_TxBuf[9]=0xFF;
		UART0_write(UART0_TxBuf,10);
		//发送失败
		ConsolePutString("can not send app data\n");
	}else{

		//SET_LED3();//~~~~
	
		UART0_TxBuf[2]=0x07;//数据长度
		UART0_TxBuf[3]=ZIGBEE_CMD_SEND;
		UART0_TxBuf[4]=1;
		//发送结果
		UART0_TxBuf[5]=0;
		//以下数据用于扩展
		UART0_TxBuf[6]=0;
		UART0_TxBuf[7]=0;
		UART0_TxBuf[8]=0;
		UART0_TxBuf[9]=0xFF;
		UART0_write(UART0_TxBuf,10);
	}

}
//帧头   |   帧长度|命令|目标端点|源端点|源地址|  data                    |结尾
// AA  FF           06               01              03                 02          MSB LSB                                      FF
// 0      1           2                 3               4                   5             6    7              8      ........           XX
void Rev_package(){
	BYTE frame_len = current_rev_data_len + 6;
	if(current_rev_flag){
		UART0_TxBuf[2] = frame_len;
		UART0_TxBuf[3] = ZIGBEE_CMD_REV;
		UART0_TxBuf[4] = current_rev_destEP;
		UART0_TxBuf[5] = current_rev_srcEP;	
		UART0_TxBuf[6] = current_rev_src_addr.byte.MSB;
		UART0_TxBuf[7] = current_rev_src_addr.byte.LSB;
		UART0_TxBuf[frame_len + 2] = 0xFF;
		UART0_write(UART0_TxBuf,frame_len + 3);
		current_rev_flag = FALSE;
	}else{
		UART0_TxBuf[2] = 0x07;
		UART0_TxBuf[3] = ZIGBEE_CMD_REV;
		UART0_TxBuf[4] = 0x00;//当前没有收到任何数据
		UART0_TxBuf[5] = 0;
		UART0_TxBuf[6] = 0;
		UART0_TxBuf[7] = 0;
		UART0_TxBuf[8] = 0;
		UART0_TxBuf[9] = 0xFF;
		UART0_write(UART0_TxBuf,10);
	}
}
// AA FF 07 10 00 0A 00 00 00 FF
//  0   1  2   3   4   5   6  7  9  10 
//本地的地址是10
void Set_address(){
	SHORT_ADDR short_addr;
	cmd_set_address();
	APLGetShortAddress(&short_addr);
	UART0_TxBuf[2]=0x07;//数据长度
	UART0_TxBuf[3]=ZIGBEE_CMD_SET_ADDR;
	UART0_TxBuf[4]=short_addr.byte.MSB;//发送结果
	UART0_TxBuf[5]=short_addr.byte.LSB;
	UART0_TxBuf[6]=0;
	UART0_TxBuf[7]=0;
	UART0_TxBuf[8]=0;
	UART0_TxBuf[9]=0xFF;
	UART0_write(UART0_TxBuf,10);
}
//设置本地EndPoint地址	
void cmd_set_current_EP(EP_HANDLE *EP){
	APLSetEP(EP[receive_data[5]]);
}
