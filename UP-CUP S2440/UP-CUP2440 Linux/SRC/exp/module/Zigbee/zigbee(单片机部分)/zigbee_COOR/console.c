#include "Console.h"

//�򿪸ú궨�壬�Ϳ���ͨ������0������Ӧ�ĵ�����Ϣ
//��Ϊ����Ҫ����ģ����Ϊһ��OEMģ����ʹ������Ҫ���ε�����
//��Ϣ
//#define UART_DEBUG


//���崮�ڵĲ����ʣ�ֹͣλ��У��λ
#define baudRate	UART_BAUDRATE_19K2
#define stop_bit	UART_OPT_ONE_STOP_BIT
#define parity		UART_OPT_NO_PARITY
#define bits		UART_OPT_8_BITS_PER_CHAR
//add by lyj
/*
*************************************************************************************************************
*          /myzigbeenewA/uart0/uart0.c
* ATmega128  UART driver
* @auther lyj
* @date 20071101
* @add new API  20071030
*************************************************************************************************************
*/
/////////////////////////////////////////////////
//                uart0����                    //
/////////////////////////////////////////////////

//��̬ȫ�ֱ���
static unsigned char UART0_RxBuf[UART0_RX_BUFFER_SIZE];
static volatile unsigned char UART0_RxHead;
static volatile unsigned char UART0_RxTail;
static unsigned char UART0_TxBuf[UART0_TX_BUFFER_SIZE];
static volatile unsigned char UART0_TxHead;
static volatile unsigned char UART0_TxTail;

static volatile unsigned char UART0_Rx_flag = 0;
//�Ƿ���յ�����

static volatile unsigned char command_valid = 0;
//���������Ƿ���Ч

static volatile unsigned char data_FCS = 0;     
//�Ƿ�У��ɹ�

static volatile unsigned char command=0x00;     
//�洢�����������

//�Ӵ��ڽ�����Ч���ݻ�����
unsigned char receive_data[125];//ȫ�ֱ���
unsigned char send_data[125];//ȫ�ֱ���

static volatile int receive_length = 0;         
//���յ�����Ч���ݳ���

static volatile int receive_count = 0;          
//��¼��ǰ֡���յ�������

static volatile int receive_a_frame = 0;        
//�ս��յ���������һ֡����

static volatile int i=0;




//���ڳ�ʼ������
void UART0_InitUART( unsigned long baudrate )
{
    unsigned char x;

    UBRR0H = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) >>8;         
	//���ò�����
    UBRR0L = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) & 0xFF;

    UCSR0B = ( (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0) );  
	//�����ڽ��ա����ͺ���������ж�
    UCSR0C = ( (1<<UCSZ01) | (1<<UCSZ00) );  
	//1λֹͣλ,8λ����λ
    x = 0;              
	//��ʼ�����ݻ���
    UART0_RxTail = x;
    UART0_RxHead = x;
    UART0_TxTail = x;
    UART0_TxHead = x;
}

//���ڽ����жϴ�����
SIGNAL(USART0_RX_vect)//18
{
    unsigned char data;
    unsigned char tmphead;
    data = UDR0;     
	//����������
	
    tmphead = ( UART0_RxHead + 1 ) & UART0_RX_BUFFER_MASK;    // 1 & 0xFF
	//���㻺������
	
    UART0_RxHead = tmphead;// 1         
	//�����µĻ�������
	
    if ( tmphead == UART0_RxTail )
    {
		//������뻺���������
		return ;
    }
	UART0_RxBuf[tmphead] = data;   
	//�ѽ������ݱ��浽���ջ���

	//��λ
	UART0_Rx_flag = 1;
	receive_count ++;
	if(receive_count >= 10){
		receive_count = 0;
		receive_a_frame = 1;
	}
}

//���з����жϴ�����
//�����ж���Ҫ�޸ĵ�
SIGNAL(USART0_UDRE_vect)//19
{
    unsigned char tmptail;
	DISABLE_GLOBAL_INT();
	

	//����Ƿ��ͻ�������������ݶ��Ѿ��������
	if ( UART0_TxHead != UART0_TxTail )  
    {
        tmptail = ( UART0_TxTail + 1 ) & UART0_TX_BUFFER_MASK;       
		//���㻺������
        UART0_TxTail = tmptail;     
		//�����µĻ�������
        UDR0 = UART0_TxBuf[tmptail];     
		//��ʼ����
    }
    else
    {
        UCSR0B &= ~(1<<UDRIE0);       
		//��UDRE�ж� 
    }
	ENABLE_GLOBAL_INT();
}

//�ӽ��ջ��������һ���ֽ�
unsigned char UART0_ReceiveByte( void )
{
    unsigned char tmptail;

    while ( UART0_RxHead == UART0_RxTail ); 
	//�ȴ���������
        
    tmptail = ( UART0_RxTail + 1 ) & UART0_RX_BUFFER_MASK;    
	//���㻺������
	
    UART0_RxTail = tmptail;       
	//�����µĻ�������
	
    return UART0_RxBuf[tmptail]; 
}

//��һ���ֽڷ��뷢�ͻ���
void UART0_TransmitByte( unsigned char data )
{
    unsigned char tmphead;
    tmphead = ( UART0_TxHead + 1 ) & UART0_TX_BUFFER_MASK;   
	//���㻺������

    while ( tmphead == UART0_TxTail );       
	//�ȴ��пյػ���ռ�

    UART0_TxBuf[tmphead] = data;     
	//�������ݵ�����

    UART0_TxHead = tmphead;     
	//�����µĻ�������

    UCSR0B |= (1<<UDRIE0); 
	//�� UDRE �ж�
}
//�жϽ��ջ����Ƿ�Ϊ��
unsigned char UART0_DataInReceiveBuffer( void )
{
    return ( UART0_RxHead != UART0_RxTail ); 
	//����0��ʾ���ջ����ѿ�
}
//add end


//���ڳ�ʼ��
void ConsoleInit(void)
{
	UART0_InitUART(115200);
}
//************************************************

void ConsolePut(BYTE c)
{
#if defined(I_AM_END_DEVICE)
	UART0_TransmitByte(c);
#endif
}
//�������д��۵ģ�һ��Ҫ��������������
void ConsolePutString(BYTE *s)
{
//add by lyj _uptech@126.com
#ifdef UART_DEBUG
    BYTE c;
    while( (c = *s++) ){
//		ConsolePut(c);
        UART0_TransmitByte(c);
		if(c == '\n')
			UART0_TransmitByte('\r');
    }
#endif	 
}
//Ŀǰ����֪��Ϊʲô,������궨��������������޷�����
#ifdef I_AM_COORDINATOR
	void CoordPut(BYTE c)
	{
			UART0_TransmitByte(c);

	}
	void CoordPutString(BYTE *s)
	{
    BYTE c;
    while( (c = *s++) )
        CoordPut(c);
	}
#endif


//���������Ӵ�ӡ���ݵĺ���
void ConsolePutInitData(int highval,int lowval)
{
#ifdef UART_DEBUG
  char i;
  BYTE dig;
  long divider;
  int wordvalue;
  wordvalue=highval*256+lowval;
  divider=10000;
  
	for(i=5; divider>wordvalue && i>1 ;i--)
	{
		divider/=10;
		UART0_TransmitByte(' ');
	}
  for(;i>0;i--)
  {

    dig = (wordvalue/divider);
	wordvalue = (wordvalue%divider);
	UART0_TransmitByte(dig+0x30);
		
    divider/=10;
  }   
  UART0_TransmitByte('\r');	
  UART0_TransmitByte('\n');	
#endif
}



void ConsolePutFlashString(PGM_P string_P){
	char *string = malloc(strlen_P(string_P)+1);
	if (string == NULL){
		ConsolePutString("malloc new char[] failed\n");
		return;
	}
	strcpy_P(string,string_P);
	ConsolePutString(string);
	free(string);
}

//************************************************
BYTE ConsoleGet(void)
{		
   	return UART0_ReceiveByte();
}

BYTE ConsoleGetString(char *buffer, BYTE bufferLen)
{
    BYTE v;
    BYTE count;
    count = 0;

    do
    {
        if ( bufferLen-- == 0 )
            break;
//		UDR1 = 0;
        
		while( !UART0_DataInReceiveBuffer() );
        v = UART0_ReceiveByte();
        if ( v == '\r' || v == '\n' )
            break;
        count++;
        *buffer++ = v;
        *buffer = '\0';
    } while(1);
    return count;
}

/******************************************
*
*  ���ߺ���
*
*
******************************************/
int UART0_read(unsigned char *buf,int nbytes){
	int i = 0;
	for(i = 0;i < nbytes;i ++){
/**
		if(!UART0_DataInReceiveBuffer()){
			halWait(50)!
			if(!UART0_DataInReceiveBuffer())
				//����Ȳ������ݾʹ��󷵻�
				return 0;
		}
*/
		buf[i] = UART0_ReceiveByte();
	}
	return i;
}
int UART0_write(unsigned char *buf,int nbytes){
	int i = 0;
	for(i = 0;i < nbytes;i ++){
		UART0_TransmitByte(buf[i]);
	}
	return i;
}
//ˢ�½��ջ���
void UART0_RX_flash(){
    UART0_RxTail = 0;
    UART0_RxHead = 0;
}
//ˢ�·��ͻ���
void UART_TX_flash(){
    UART0_TxTail = 0;
    UART0_TxHead = 0;
}
/***********************************************************
*
*֡ͷ   |   ֡����|����|Ŀ��˵�|Դ�˵�|Ŀ���ַ|     ����| ���� |��β
* AA  FF           06               00              03                 02                MSB LSB                                       FF
* 0      1           2                 3               4                   5                  6    7                8            9           10
*
*
***********************************************************/
int decode_rec(){
	if(!UART0_Rx_flag)
		//��û���յ�����
		return 0;
	//��λ���У��λ
	data_FCS = 0;
	//step 0 ��ȡһ֡����
	//�ȶ�ȡ֡ͷ
//	if((receive_data[0] = UART0_ReceiveByte()) != 0xAA)
//		return 0;
//		goto DECODE_FAIL;//���ݲ������
//��ֹǰ�滹������û�ж�������
	for(;;){
		//�������е����ݻ���������ƥ��
		if(UART0_DataInReceiveBuffer()){
			if((receive_data[0] = UART0_ReceiveByte()) == 0xAA)
				if((receive_data[1] = UART0_ReceiveByte()) == 0xFF)
					{
						//׷��һ���ж�
						break;
					}
		}else
			goto DECODE_FAIL;//���ݲ������
	}
	//�õȴ���Ϊ�˷�ֹǰ����������ӵ�
	//���ǳ�����ϵͳ�ȴ�������
//	while((receive_data[0] = UART0_ReceiveByte()) != 0xAA);

	//֡ͷ��ȷ�Ļ�����ʣ�����ݶ���
	for(i = 2;i < 11;i ++ ){//�����ݸ��ĵ��ӵ�������ʼ��ȡ
		while(!UART0_DataInReceiveBuffer());
		if(UART0_DataInReceiveBuffer()){
			receive_data[i] = UART0_ReceiveByte(); 
		}else
			goto DECODE_FAIL;
			//���ݲ������
	}
		
	//step1
	//��һ���ֽ�
	if(receive_data[0] != 0xAA)
		goto DECODE_FAIL;
	//�ڶ����ֽ�
	if(receive_data[1] != 0xFF)
		goto DECODE_FAIL;
/*
	//step2
	//��ȡ����
	if(receive_data[3] != 0x01)
		goto DECODE_FAIL;
	if(receive_data[4] != 0xA2)
		goto DECODE_FAIL;

*/
	//step3
	//����β
	//������һ������У��λ���ڸ�Ϊ0xff
	//��������У��Ĺ���
	//���㽫����չ
	if(receive_data[10] != 0xFF)
		goto DECODE_FAIL;
	command_valid = 1;
	command=receive_data[3];
	//ˢ�½��ջ���
	UART0_RX_flash();	
	//�����־λ
	UART0_Rx_flag = 0;
	return receive_length;
DECODE_FAIL:
	command_valid = 0;
	//ˢ�½��ջ���
	UART0_RX_flash();	
	//�����־λ
	UART0_Rx_flag = 0;
	for(i=0;i < 11;i ++)
		receive_data[i] = 0;
	return 0;
}
void test_for(){
	UART0_write(receive_data,11);
}

unsigned char if_receive_a_frame(){
	return receive_a_frame ;
}
void Reset_receive_frame(){
	receive_a_frame = 0;
}
unsigned char if_command_valid(){
	return command_valid;
}
unsigned char get_command(){
	return command;
}
void Reset_command_valid(){
	command_valid = 0;
}
unsigned char get_rx_data(int index){
	return receive_data[index];
}


