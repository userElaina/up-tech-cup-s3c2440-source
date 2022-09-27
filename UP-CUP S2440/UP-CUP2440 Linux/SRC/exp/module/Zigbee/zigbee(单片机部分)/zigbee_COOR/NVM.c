#include "NVM.h"

EEMEM SHORT_ADDR macShortAddr = {0xff, 0xff};

EEMEM LONG_ADDR macLongAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void NVMWrite(void *dest, const void *src, size_t count){
	eeprom_write_block (src, dest, count);
}

void NVMRead(void *dest, const void *src, size_t count){

	eeprom_read_block (dest, src, count);
}

void ClearNVM( void *dest, BYTE count )
{
    BYTE    i;
    BYTE    dummy = 0;

    for (i=0; i<count; i++)
        NVMWrite( dest, &dummy, 1 );
}
