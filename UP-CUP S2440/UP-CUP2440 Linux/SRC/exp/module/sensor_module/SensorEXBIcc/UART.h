#ifndef  _UART_H_
#define _UART_H_

extern unsigned char SendBuffer[7];

void USART_Init(void);
void UARTSendaByte(unsigned char cTransmitValue);
unsigned char UARTRcvaByte(void);
unsigned char WaitForCMD(void);
void UARTSendAMessage(unsigned char *SendBuffer, unsigned int num);

#endif 