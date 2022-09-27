/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2004.4.26
\***************************************************************************/
	

/***************************************************************************\
    #说明: 驱动程序定义
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------
	2004-4-26	添加串口驱动程序s3c2440_serialx_driver

\***************************************************************************/

#include "../inc/macro.h"
#include "../inc/drivers.h"
//#include "../inc/sys/sysdrv.h"
#include "uhal/uart.h"

/***************************************************************************\
	for serial drivers
\***************************************************************************/
extern serial_driver_t s3c2440_serial0_driver, s3c2440_serial1_driver, 
		s3c2440_serial2_driver;
serial_driver_t* serial_drv[]={&s3c2440_serial0_driver, &s3c2440_serial1_driver, 
		&s3c2440_serial2_driver};
int NumberOfUartDrv=NumberOfArray(serial_drv);
//struct_Uart_Buffer	Uart_Buffer[NumberOfArray(serial_drv)];




