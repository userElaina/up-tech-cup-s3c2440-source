#include "NVM.h"

EEMEM SHORT_ADDR macShortAddr = {0xff, 0xff};

EEMEM LONG_ADDR macLongAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void NVMWrite(void *dest, const void *src, size_t count){
	eeprom_write_block (src, dest, count);
}

void NVMRead(void *dest, const void *src, size_t count){

	eeprom_read_block (dest, src, count);
}

/*
void eeprom_read_block (void *pointer_ram, const void *pointer_eeprom,size_t n)
{
	if (!__builtin_constant_p (n)|| n > 256)
	{
		//make sure size is a 16 bit variable.
		uint16_t size = n; 
		 __asm__ __volatile__ ( 
								".%=_start:" CR_TAB
            							"sbiw %2,1" CR_TAB
           							"brlt .%=_finished" CR_TAB
             							XCALL " __eeprom_read_byte_" _REG_LOCATION_SUFFIX CR_TAB
            							"st z+,__tmp_reg__" CR_TAB
            							"rjmp .%=_start" CR_TAB
            							".%=_finished:" 
          							: "=x" (pointer_eeprom),
            							"=z" (pointer_ram),
            							"+w" (size)
           							: "x" (pointer_eeprom), 
            							 "z" (pointer_ram)
           							: "memory");
	}
	else
    	{
      		if (n != 0)
        	{
          		if (n == 256)
            		{
              		__asm__ __volatile__ (
                  								XCALL " __eeprom_read_block_" _REG_LOCATION_SUFFIX 
                								: "+x" (pointer_eeprom),
                  								"=z" (pointer_ram)
                								: "z"  (pointer_ram)
                								: "memory");
            		}
          		else
            		{
             			 // Needed in order to truncate to 8 bit. 
              		uint8_t len;
              		len = (uint8_t) n; 
				 __asm__ __volatile__ (
                  								"mov __zero_reg__,%2"      CR_TAB
                   								XCALL " __eeprom_read_block_" _REG_LOCATION_SUFFIX 
               		 						: "+x" (pointer_eeprom),
                  								"=z" (pointer_ram)
                								: "r"  (len),
                  								"z"  (pointer_ram)
                								: "memory");
            		}
        	}
    	}
}
*/



void ClearNVM( void *dest, BYTE count )
{
    BYTE    i;
    BYTE    dummy = 0;

    for (i=0; i<count; i++)
        NVMWrite( dest, &dummy, 1 );
}
