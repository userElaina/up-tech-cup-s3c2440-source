#include "../inc/drivers.h"
#include "../inc/lib.h"
#include <string.h>
#include <stdio.h>

#include "led.h"                 

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 


int main(void)
{
	
	ARMTargetInit();	//开发版初始化
	set_lednum();
	test_led();   



 	return 0;
}

	
	

