#include "../inc/drivers.h"
#include "../inc/lib.h"
#include <string.h>
#include <stdio.h>

#include "led.h"                 

#pragma import(__use_no_semihosting_swi)  // ensure no functions that use semihosting 


int main(void)
{
	
	ARMTargetInit();	//�������ʼ��
	set_lednum();
	test_led();   



 	return 0;
}

	
	

