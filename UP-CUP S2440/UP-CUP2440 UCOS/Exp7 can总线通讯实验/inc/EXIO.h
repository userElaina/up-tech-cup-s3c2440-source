
#define EXIOADDR		(*(volatile unsigned short*)0xa000000)		//bank5

unsigned short int EXIOReg;

#define SETEXIOBIT(bit)		do{EXIOReg|=bit; 	EXIOADDR=EXIOReg;}while(0)
#define CLREXIOBIT(bit)		do{EXIOReg&=(~bit); 	EXIOADDR=EXIOReg;}while(0)
#define SETEXIOBITMASK(bit,mask)	do{EXIOReg&=(~mask); EXIOReg|=bit; 	EXIOADDR=EXIOReg;}while(0)