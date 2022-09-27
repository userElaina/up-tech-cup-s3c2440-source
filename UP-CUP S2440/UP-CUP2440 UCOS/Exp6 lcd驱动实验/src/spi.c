/***************************************************************************\
	Copyright (c) 2004-2007 threewater@up-tech.com, All rights reserved.
	by threewter	2005.2.26
\***************************************************************************/
/***************************************************************************\
    #说明: spi接口驱动程序
	----------------------------------  Bug  --------------------------------------

	----------------------------------  TODO list  --------------------------------------

	----------------------------------修正--------------------------------------

	2005-2-26	创建

\***************************************************************************/

//#include "../ucos-ii/includes.h"
#include "../inc/spi.h"
#include "../inc/reg2410.h"
#include "../inc/lib.h"
#include "../inc/bitfield.h"

#define GPIO_TC_SPIMISO		(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E11)
#define GPIO_TC_SPIMOSI		(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E12)
#define GPIO_TC_SPICLK		(GPIO_MODE_ALT0 | GPIO_PULLUP_DIS | GPIO_E13)

void SPISend ( unsigned char val, int channel )
{


	if ( channel == 0 ){
		rSPTDAT0 = val;
		while (!(rSPSTA0 & 1));
	}else if ( channel == 1 ){
		rSPTDAT1 = val;
		while (!(rSPSTA1 & 1));
	}
	
	
}

unsigned char SPIRecv ( int channel )
{
	if (channel == 0){
		return rSPRDAT0;
	}else if (channel == 1) {
		return rSPRDAT1;
	}else
		return 0xff;
}

void Set_SIO_mode(int channel, int nSPCON, int nSPPRE, int SPPIN,
		int *poldnSPCON, int *poldnSPPRE, int *poldSPPIN)
{	
	if(channel==0){
		if(poldnSPCON){
			*poldnSPCON=rSPCON0;
		}
		rSPCON0=nSPCON;

		if(poldnSPPRE){
			*poldnSPPRE=rSPPRE0;
		}
		rSPPRE0=nSPPRE;

		if(poldSPPIN){
			*poldSPPIN=rSPPIN0;
		}
		rSPPIN0=SPPIN;
	}else if(channel == 1){
		if(poldnSPCON){
			*poldnSPCON=rSPCON1;
		}
		rSPCON1=nSPCON;

		if(poldnSPPRE){
			*poldnSPPRE=rSPPRE1;
		}
		rSPPRE1=nSPPRE;

		if(poldSPPIN){
			*poldSPPIN=rSPPIN1;
		}
		rSPPIN1=SPPIN;
	}else 
		return;

	hudelay(1);
}

void SPI_initIO(int channel)
{
	if (channel == 0){
		set_gpio_ctrl (GPIO_TC_SPIMISO);
		set_gpio_ctrl (GPIO_TC_SPIMOSI);
		set_gpio_ctrl (GPIO_TC_SPICLK);
	}else if (channel == 1) {

	}
}

void SPI_init(void)
{SPI_initIO(0);
	Set_SIO_mode(0, SPCON_SMOD_POLL | 
		SPCON_ENSCK | SPCON_MSTR |SPCON_CPOL_HIGH | 
		SPCON_CPHA_FMTA, 33, 2, NULL, NULL, NULL);
}