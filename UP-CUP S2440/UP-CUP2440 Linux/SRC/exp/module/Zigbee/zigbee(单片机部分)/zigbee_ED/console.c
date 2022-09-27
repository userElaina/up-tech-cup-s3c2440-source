#include "Console.h"

//���崮�ڵĲ����ʣ�ֹͣλ��У��λ
#define baudRate	UART_BAUDRATE_19K2
#define stop_bit	UART_OPT_ONE_STOP_BIT
#define parity		UART_OPT_NO_PARITY
#define bits		UART_OPT_8_BITS_PER_CHAR
//add by lyj
//#define UART_DEBUG
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
static volatile unsigned char command_valid = 0;
static volatile unsigned char data_FCS = 0;

//���ڳ�ʼ������
void UART0_InitUART( unsigned long baudrate )
{
    unsigned char x;

    UBRR0H = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) >>8;         //���ò�����
    UBRR0L = (unsigned char)(CPU_CLK_FREQ/(16*baudrate) - 1) & 0xFF;

    UCSR0B = ( (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0) );  //�����ڽ��ա����ͺ���������ж�
    UCSR0C = ( (1<<UCSZ01) | (1<<UCSZ00) );  //1λֹͣλ,8λ����λ
    x = 0;              //��ʼ�����ݻ���
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
    data = UDR0;     //����������
    tmphead = ( UART0_RxHead + 1 ) & UART0_RX_BUFFER_MASK;    //���㻺������
    UART0_RxHead = tmphead;         //�����µĻ�������
    if ( tmphead == UART0_RxTail )
    {
        return ;//������뻺���������
    }
    UART0_RxBuf[tmphead] = data;   //�ѽ������ݱ��浽���ջ���
	//��λ
	UART0_Rx_flag = 1;
}

//���з����жϴ�����
//�����ж���Ҫ�޸ĵ�
SIGNAL(USART0_UDRE_vect)//19
{
    unsigned char tmptail;

    if ( UART0_TxHead != UART0_TxTail )  //����Ƿ��ͻ�������������ݶ��Ѿ��������
    {
        tmptail = ( UART0_TxTail + 1 ) & UART0_TX_BUFFER_MASK;       //���㻺������
        UART0_TxTail = tmptail;     //�����µĻ�������
        UDR0 = UART0_TxBuf[tmptail];     //��ʼ����
    }
    else
    {
        UCSR0B &= ~(1<<UDRIE0);       //��UDRE�ж� 
    }
}

//�ӽ��ջ��������һ���ֽ�
unsigned char UART0_ReceiveByte( void )
{
    unsigned char tmptail;

    while ( UART0_RxHead == UART0_RxTail ); //�ȴ���������
        
    tmptail = ( UART0_RxTail + 1 ) & UART0_RX_BUFFER_MASK;    //���㻺������
    UART0_RxTail = tmptail;       //�����µĻ�������
    return UART0_RxBuf[tmptail]; 
}

//��һ���ֽڷ��뷢�ͻ���
void UART0_TransmitByte( unsigned char data )
{
//#ifdef DEBUG_UPTECH

    unsigned char tmphead;
    tmphead = ( UART0_TxHead + 1 ) & UART0_TX_BUFFER_MASK;   //���㻺������

    while ( tmphead == UART0_TxTail );       //�ȴ��пյػ���ռ�
        
    UART0_TxBuf[tmphead] = data;     //�������ݵ�����
    UART0_TxHead = tmphead;     //�����µĻ�������
    UCSR0B |= (1<<UDRIE0); //�� UDRE �ж�
//#endif
}
//�жϽ��ջ����Ƿ�Ϊ��
unsigned char UART0_DataInReceiveBuffer( void )
{
    return ( UART0_RxHead != UART0_RxTail ); //����0��ʾ���ջ����ѿ�
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
#ifdef UART_DEBUG
	BYTE c;
    while( (c = *s++) ){
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



