#include "tty.h"
#include "stdio.h"
#undef uchar
#define uchar unsigned char
void zigbee_rev_data(uchar *addr,uchar n);



//ÉèÖÃ×ó±ß½çi
//aa ff 08 00 03 02 00 0C A1 01 ff
void zigbee_set_LED(uchar *addr,uchar cmd,uchar arg);


void zigbee_set_default();
