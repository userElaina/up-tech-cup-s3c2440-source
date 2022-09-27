#define GPGCON	(*(volatile unsigned int*)(0x56000060))
#define GPGDAT	(*(volatile unsigned int*)(0x56000064))
#define GPGUP     (*(volatile unsigned int*)(0x56000068))
#define MAX504_ENABLE()		       do{GPGDAT &=~(0x1<<3);}while(0)
#define MAX504_DISABLE()		do{GPGDAT |=(0x1<<3);}while(0)
void Max504_SetDA(int value);
void DA_Test(void);
void setgpio(void);

