#include "zigbee.h"		// Zigbee����
#include "zAPL.h"	//ZigbeeӦ�ò�	

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
//���������
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

//�ص�����
//MAC�㷢��FRAMEʱ����
void AppMACFrameTransmitted(void);
//MAC��ӵ�FRAMEʱ����
void AppMACFrameReceived(void);
//FRAMEӦ��ʱʱ����
void AppMACFrameTimeOutOccurred(void);
//ѡ���ŵ�ʱ����
BOOL AppOkayToUseChannel(BYTE channel);
//����RFD�������ʱ����
BOOL AppOkayToAcceptThisNode(LONG_ADDR *longAddr);
//��RFD����󴥷�
void AppNewNodeJoined(LONG_ADDR *nodeAddr, BOOL bIsRejoined);
//��RFD�뿪���紥��
void AppNodeLeft(LONG_ADDR *nodeAddr);

//Ӳ����ʼ����
static void InitializeBoard(void);
static unsigned char UART0_TxBuf[128]={0xBB,0xFF,0x06,0x00,0xA2,0x00,0x00,0x00,0x00,0xFF};
//�Ӵ��ڽ�����Ч���ݻ�����
static volatile unsigned char receive_package[125];
static volatile unsigned char send_package[125];
static volatile SHORT_ADDR current_rev_dest_addr;
static volatile SHORT_ADDR current_rev_src_addr;
//���յ�������ȫ�����أ���Ӧ�ó���˽���
static volatile BYTE current_rev_data_len;//���ص����ݳ�������֪��
static volatile BYTE current_rev_srcEP;
static volatile BYTE current_rev_destEP;
static volatile BYTE current_send_dest_addr;
static volatile BYTE current_send_src_addr;
static volatile BYTE current_send_data_len;//���͵�����ֻ��һ���ֽ�
static volatile BYTE current_send_cmd;//ȫ��ʹ�õ���cmd
static volatile BYTE current_send_params;
//
//��������
#define ZIGBEE_CMD_SEND 0x00
#define ZIGBEE_CMD_REV  0x01
#define ZIGBEE_CMD_SET_ADDR 0x10


//flag ��־λ
//�Ƿ��յ�����
static volatile BOOL current_rev_flag = FALSE;
static int i;


int main(void)
{
    //��ַ���ÿ���
	BOOL bInConfigMode = FALSE;
	
	BYTE receive_length;

	BYTE data[2] = {0,0};
	
	EP_HANDLE EP[NUM_DEFINED_ENDPOINTS],current_EP;

	BYTE dest_addr,src_addr;

	//Э������ַ
	SHORT_ADDR Rfd_dest;

	//����ID
	TRANS_ID trans_id = 0;

	//������
	BOOL send_lock = FALSE;
    enum
    {
        SM_APP_INIT = 0,
        SM_APP_INIT_RUN,
        SM_APP_NORMAL_START,
        SM_APP_NORMAL_RUN
    } smApp;
	
	//��ʼ��Ӳ��
	InitializeBoard();
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

//��һ��ʹ�ø�ģ��ʱ��Ҫ��Ҫ�������õ�ַ��Ĭ�ϵĵ�ַ��0x10;
//��ȡEEPROM�е����ݣ��ж��Ƿ��Ѿ������˵�ַ
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
		//���ó���ַ
        APLPutLongAddress(&longAddr);
		//���ö̵�ַ
		APLPutShortAddress(&short_addr);
		//����CC2420��ַ
        APLUpdateAddressInfo();
    }

	//��ʼ��Э��ջ
    APLInit();
	//ʹ��Э��ջ 
    APLEnable();
	//��������  
    APLNetworkInit();

    smApp = SM_APP_INIT;
	
	
    while (1)
    {
//    	ConsolePutString(__FUNCTION__);
//		ConsolePutString("\r\n");
        //APLTask()��Э��ջ������������frame�����뱣֤����������񣬷����ܽ�������
        APLTask();

        switch (smApp)
        {
			case SM_APP_INIT:
				ConsolePutString("Starting a new network...\r\n");
				smApp = SM_APP_INIT_RUN;

				//SET_LED3();//~~~~
				
				break;

			case SM_APP_INIT_RUN:
				//�����Ƿ������,ֻ�е���GetLastZError()����ZCODE_NO_ERRORʱ���ű����ɹ�����
				if ( APLIsNetworkInitComplete() )
				{
					if ( GetLastZError() == ZCODE_NO_ERROR )
					{
						ConsolePutString("New network successfully started.\r\n");
						//��������
						APLNetworkForm();
						smApp = SM_APP_NORMAL_START;
					}
					else
					{
						//���罨��ʧ�ܣ����½���
						ConsolePutString("There was an error starting network.\r\n");
						smApp = SM_APP_INIT;
					}
				}
				break;

			case SM_APP_NORMAL_START:
				//����RFD��������
				//SET_LED3();//~~~~
				APLPermitAssociation();
				ConsolePutString("SM_APP_NORMAL_START: permit join.\r\n");
				smApp = SM_APP_NORMAL_RUN;

				//SET_LED2();//~~~~
				
				break;

			case SM_APP_NORMAL_RUN:
				//��2�Ŷ˵㣬epΪָ��2�Ŷ˵�ľ��
				for(i = 0;i <NUM_DEFINED_ENDPOINTS;i ++)         //0~4
					EP[i] = APLOpenEP(i);
				//Ĭ������2�Ŷ˵�Ϊ��ǰ�˵�
				current_EP = EP[2];
				//Ӧ����ѭ��
				while(1){
					//SET_LED3();//~~~~
					APLSetEP(current_EP);
					APLTask();
					//SET_LED3();//~~~~
#if 1
					//�Ƿ���յ�����
					if ( APLIsGetReady() )//~~~~~~~~~~~~~~~~~~~~~~~~~~~
					{

						//SET_LED3();//~~~~

						ConsolePutString("receive data..\n");
						//��һ���ֽ���Ч
						APLGet();
						//���ݰ�����
						receive_length = APLGet();
						current_rev_data_len = receive_length;
						//ͨ�����ڽ��������ݴ�ӡ����
						for(i = 0;i < receive_length;i ++){
							UART0_TxBuf[8 + i] = APLGet();
							//ֱ�Ӵ���ڴ��ڷ��͵����ݻ�����
							
							//for test
							//UART0_TransmitByte(UART0_TxBuf[8 + i]);
	
						}

						current_rev_dest_addr = APLGetDestShortAddr();
						current_rev_src_addr = APLGetSrcShortAddr();



						
					//������һ�����ݰ�����붪���������ܽ����µ����ݰ�
						APLDiscardRx();
						current_rev_flag = TRUE;
					}
#endif
					//������������
					//receive_a_frame   �ڴ��ڽ����ж��ﱻ�޸���
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
							//����������󣬷�������
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
					//LED 1��������ָʾ��
					SET_LED1();
					halWait(500);
					CLR_LED1_EN();
				}

				break;
        }
    }
	return 1;
}






//�ص�������ʵ�֣��û����Լ������
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
//��ʼ����ʱ�򲻱س�ʼ��cc2420����ض��壬ϵͳ�ڵ���
//������ʱ����Զ�������ص��߼�
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
/******************************************
*
*
* ���ߺ���������Ϊ�������ļ�������
*
*
*******************************************/
//�Ӵ��ڵ�����������6���ַ������� ��00000\����ʾ��ַ0, ��00001\����ʾ��ַ1
//char idString[6];
// AA FF 06 10 00 0A 00 00 FF
//  0   1  2   3   4   5   6  7  9
//���صĵ�ַ��10
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
	//���ó���ַ
    APLPutLongAddress(&longAddr);
	//���ö̵�ַ
	APLPutShortAddress(&short_addr);
	//����CC2420��ַ
    APLUpdateAddressInfo();   
}




//�����û���������Ϣ����ȡ���÷�������֡
//��Ҫ��������ת��Ϊʮ����
//����֡��ʽ
// AA FF 06 00 03 02 00 0C 34 35 FF
//����������Ŀ�ĵ�ַ(XX XX)��Ŀ�Ķ˵�(XX)��Դ�˵�(XX)������(XX)
//���ط��ͽ��
//֡ͷ   |   ֡����|����|Ŀ��˵�|Դ�˵�|Ŀ���ַ|     ����| ���� |��β
// AA  FF           06               00              03                 02                MSB LSB                                       FF
// 0      1           2                 3               4                   5                  6    7                8            9           10
void cmd_set_send_info(){
		SHORT_ADDR Rfd_dest;
		BYTE *pointer = "ab";
		//ע���ڷ��͵�ʱ��ҲҪʹ��������
		Rfd_dest.byte.MSB = receive_data[6];
		Rfd_dest.byte.LSB = receive_data[7];
		pointer[0] = receive_data[8];
		pointer[1] = receive_data[9];
		//���ò��ø���
		currentMessageInfo.flags.Val = APS_MESSAGE_DIRECT |         \
										   APS_MESSAGE_SHORT_ADDRESS |  \
										   APS_MESSAGE_NO_ACK;          \
		currentMessageInfo.frameType              = MSG_FRAME;      
		currentMessageInfo.profileID              = MY_PROFILE_ID;            
		currentMessageInfo.clusterID              = PICDEMZ_DEMO_CLUSTER_ID;            
		//Դ�ڵ�˵��
		currentMessageInfo.srcEP                  = receive_data[5];    
		//Ŀ�Ľڵ��ַ����ΪЭ����
		currentMessageInfo.destAddress.shortAddr  = Rfd_dest;   
		//Ŀ�Ľڵ�˵��,������Ŀ�Ľڵ�˵�ţ�������ܲ�������
		currentMessageInfo.destEP                 = receive_data[4];  //modify by lyj
		//���ݰ�����
		currentMessageInfo.dataLength             = strlen(pointer);             
		//���ݰ�����
		currentMessageInfo.dataPointer            = pointer;

		currentMessageInfo.transactionID          = GENERATE_TRANS_ID;
}
// AA FF 06 01 XX XX XX XX FF
//��ȡ��ǰ֡����
//����֡��ʽ
// BB FF XX 01 
//�������� : ���͵�Դ��ַ(shortaddr)(XX XX)��Դ�˵�(XX)��Ŀ��˵�(XX)��
//���ص�����(XX XX XX XX XX XX)
void cmd_set_rev_info(BYTE *data)
{
	
}

//0xBB,0xFF,0x06,0x00,0xA2,0x00,0x00,0x00,0x00,0xFF
//BB FF 07 00 00 00 00 00 00 FF
//0    1   2  3  4   5  6   7  8   9
void Send_package(EP_HANDLE *End_Point)
{
	//����ID
	TRANS_ID trans_id = 0;
	APLSetEP(End_Point[receive_data[5]]);
	//�Ҵ���
	
	cmd_set_send_info();
	//��������
	trans_id = APLSendMessage( &currentMessageInfo );

	//SET_LED3();//~~~~
	
	if (trans_id == TRANS_ID_INVALID)
	{

		//SET_LED3();//~~~~

		UART0_TxBuf[2]=0x07;//���ݳ���
		UART0_TxBuf[3]=ZIGBEE_CMD_SEND;
		UART0_TxBuf[4]=0;//���ͽ��
		UART0_TxBuf[5]=0;
		UART0_TxBuf[6]=0;
		UART0_TxBuf[7]=0;
		UART0_TxBuf[8]=0;
		UART0_TxBuf[9]=0xFF;
		UART0_write(UART0_TxBuf,10);
		//����ʧ��
		ConsolePutString("can not send app data\n");
	}else{

		//SET_LED3();//~~~~
	
		UART0_TxBuf[2]=0x07;//���ݳ���
		UART0_TxBuf[3]=ZIGBEE_CMD_SEND;
		UART0_TxBuf[4]=1;
		//���ͽ��
		UART0_TxBuf[5]=0;
		//��������������չ
		UART0_TxBuf[6]=0;
		UART0_TxBuf[7]=0;
		UART0_TxBuf[8]=0;
		UART0_TxBuf[9]=0xFF;
		UART0_write(UART0_TxBuf,10);
	}

}
//֡ͷ   |   ֡����|����|Ŀ��˵�|Դ�˵�|Դ��ַ|  data                    |��β
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
		UART0_TxBuf[4] = 0x00;//��ǰû���յ��κ�����
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
//���صĵ�ַ��10
void Set_address(){
	SHORT_ADDR short_addr;
	cmd_set_address();
	APLGetShortAddress(&short_addr);
	UART0_TxBuf[2]=0x07;//���ݳ���
	UART0_TxBuf[3]=ZIGBEE_CMD_SET_ADDR;
	UART0_TxBuf[4]=short_addr.byte.MSB;//���ͽ��
	UART0_TxBuf[5]=short_addr.byte.LSB;
	UART0_TxBuf[6]=0;
	UART0_TxBuf[7]=0;
	UART0_TxBuf[8]=0;
	UART0_TxBuf[9]=0xFF;
	UART0_write(UART0_TxBuf,10);
}
//���ñ���EndPoint��ַ	
void cmd_set_current_EP(EP_HANDLE *EP){
	APLSetEP(EP[receive_data[5]]);
}
