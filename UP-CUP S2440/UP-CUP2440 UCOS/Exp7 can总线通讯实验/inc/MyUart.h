//void Uart_Init(int Uartnum, int mclk,int baud);
//void Uart_SendByte(int Uartnum, unsigned char data);
//void Uart_Printf(char *fmt,...);
char Uart_Getch(char* Revdata, int Uartnum, int timeout);
//void Uart_TxEmpty(int Uartnum);
int Uart_Poll(int Uartnum);