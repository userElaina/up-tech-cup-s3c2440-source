/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:

    cs8900.c

Abstract:

    Card-specific functions for the NDIS 3.0 CS8900 driver.
    
Updates:

	2003.10.24	jylee	changes on interrupt registration for NDIS
	

--*/

#include "precomp.h"


// CS8900 signature read from PacketPage Pointer port at reset
#define CS8900_SIGNATURE                0x3000
// CS8900 EISA number
#define CS8900_EISA_NUMBER              0x630e
// CS8900 product ID
#define CS8900_PRDCT_ID                 0x0000
#define CS8900_PRDCT_ID_MASK            0xe0ff

#define PKTPG_EISA_NUMBER               0x0000
#define PKTPG_PRDCT_ID_CODE             0x0002

volatile unsigned char *EthCommand;
volatile S3C2440A_INTR_REG *v_pINTRegs;
volatile S3C2440A_IOPORT_REG *v_pIOPRegs;
volatile S3C2440A_MEMCTRL_REG *v_pMEMRegs;

unsigned short GetD_SlowIO(unsigned long regaddrs)
{
	return (*(volatile unsigned short *)regaddrs);
}

void SetD_SlowIO(unsigned long regaddrs, unsigned short regvalue)
{
	*(volatile unsigned short *)regaddrs = regvalue;
}

//Read data from a PacketPage register.
unsigned short CS8900ReadRegister(unsigned short offset)
{
	writeIoPort(IO_PACKET_PAGE_POINTER, offset);
	return readIoPort(IO_PACKET_PAGE_DATA_0);
}

// Write data to a PacketPage register.
void CS8900WriteRegister(unsigned short offset, unsigned short data)
{
	writeIoPort(IO_PACKET_PAGE_POINTER, offset);
	writeIoPort(IO_PACKET_PAGE_DATA_0, data);
}

int findCS(void)
{
	unsigned short sg;

	/* Check the signature. */
	if ((sg = readIoPort(IO_PACKET_PAGE_POINTER)) != CS8900_SIGNATURE)
	{
		DEBUGMSG(1, (TEXT("Signature Error %x\n"), sg));
//		return FALSE;
	}

	/* Check the EISA registration number. */
	if (CS8900ReadRegister(PKTPG_EISA_NUMBER) != CS8900_EISA_NUMBER)
	{
		DEBUGMSG(1, (TEXT("Eisa Number Error\n")));
		return FALSE;
	}

	/* Check the Product ID. */
	if ((CS8900ReadRegister(PKTPG_PRDCT_ID_CODE) & CS8900_PRDCT_ID_MASK)
		!= CS8900_PRDCT_ID)
	{
		DEBUGMSG(1, (TEXT("Product ID Error\n")));
		return FALSE;
	}

	DEBUGMSG(1, (TEXT("Product ID = %x\n"), CS8900ReadRegister(PKTPG_PRDCT_ID_CODE)));


	return TRUE;
}

//Reset CS8900 chip.
#define MAX_COUNT	0x100000
int resetCS(void)
{
	int i;
	unsigned short dummy;

    // nGCS3=nUB/nLB(nSBHE),nWAIT,16-bit
    v_pMEMRegs->BWSCON = (v_pMEMRegs->BWSCON&~(0xf<<12))|(0xd<<12);	
    // BANK3 access timing
	v_pMEMRegs->BANKCON3=((CS8900_Tacs<<13)+(CS8900_Tcos<<11)+(CS8900_Tacc<<8)+(CS8900_Tcoh<<6)\
		+(CS8900_Tah<<4)+(CS8900_Tacp<<2)+(CS8900_PMC));
	v_pIOPRegs->EXTINT1 = (v_pIOPRegs->EXTINT1 & ~(7<<4)) | (4<<4);

    /* Issue a reset command to the chip */	
    CS8900WriteRegister(PKTPG_SELF_CTL, SELF_CTL_RESET);

    /* Delay for 125 micro-seconds */
	Sleep(10);

    /* Transition SBHE to switch chip from 8-bit to 16-bit */

#if	0
	*(volatile unsigned char *)&EthCommand[IO_PACKET_PAGE_POINTER];
	*(volatile unsigned char *)&EthCommand[IO_PACKET_PAGE_POINTER+1];
	*(volatile unsigned char *)&EthCommand[IO_PACKET_PAGE_POINTER];
	*(volatile unsigned char *)&EthCommand[IO_PACKET_PAGE_POINTER+1];
#endif

    /* Wait until initialization is done */
	Sleep(100);

	/* Wait until INITD bit of SelfST register is set. */
	i = 0;
	while (((dummy = CS8900ReadRegister(PKTPG_SELF_ST)) &
		SELF_ST_INITD) == 0)
	{
		if (++i > MAX_COUNT)
		{
			DEBUGMSG(1, (TEXT("resetCs8900 failed 1\n")));
			return FALSE;
		}
	}

	/* Wait until SIBUSY bit of SelfST register is cleared. */
	i = 0;
	while (((dummy = CS8900ReadRegister(PKTPG_SELF_ST)) &
		SELF_ST_SIBUSY) != 0)
	{
		if (++i > MAX_COUNT)
		{
			DEBUGMSG(1, (TEXT("resetCs8900 failed 2\n")));
			return FALSE;
		}
	}

	return TRUE;
}

void initIrq(void)
{
	unsigned short rdata;

	// interrupt enabling....  
	CS8900WriteRegister(PKTPG_INTERRUPT_NUMBER,INTERRUPT_NUMBER);
        
	rdata = CS8900ReadRegister(PKTPG_BUS_CTL) | BUS_CTL_ENABLE_IRQ;
	CS8900WriteRegister(PKTPG_BUS_CTL, rdata);

	rdata = CS8900ReadRegister(PKTPG_LINE_CTL) | LINE_CTL_RX_ON | LINE_CTL_TX_ON;

	CS8900WriteRegister(PKTPG_LINE_CTL, rdata);
}


//Initialize CS8900 chip.
int initCS()
{	

	CS8900WriteRegister(PKTPG_LINE_CTL, LINE_CTL_10_BASE_T);

	CS8900WriteRegister(PKTPG_RX_CFG, RX_CFG_RX_OK_I_E);

	CS8900WriteRegister(PKTPG_RX_CTL,
		RX_CTL_RX_OK_A | RX_CTL_IND_ADDR_A |
		RX_CTL_BROADCAST_A);

	CS8900WriteRegister(PKTPG_TX_CFG, 0); 

	CS8900WriteRegister(PKTPG_BUF_CFG, 0); 
	
	CS8900WriteRegister(PKTPG_INDIVISUAL_ADDR, 0x3322);
	CS8900WriteRegister(PKTPG_INDIVISUAL_ADDR + 2, 0x5544);
	CS8900WriteRegister(PKTPG_INDIVISUAL_ADDR + 4, 0x0F66);

	initIrq();

	return TRUE;
}

int CSInit()
{
	// Find CS8900 chip. 
	if (findCS() == FALSE)
		return FALSE;

	DEBUGMSG(1, (TEXT("Find CS8900 OK\r\n")));

	/* Reset CS8900 chip. */
	if (resetCS() == FALSE)
		return FALSE;

	DEBUGMSG(1, (TEXT("reset CS8900 OK\r\n")));

	/* Initialize CS8900 chip. */
	if (initCS() == FALSE)
		return FALSE;

	DEBUGMSG(1, (TEXT("CS8900 init OK\r\n")));

	return TRUE;		
}



#pragma NDIS_PAGEABLE_FUNCTION(CS8900Initialize)

BOOLEAN
CS8900Initialize(
    IN PCS8900_ADAPTER Adapter
    )

/*++

Routine Description:

    Initializes the card into a running state.

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    TRUE, if all goes well, else FALSE.

--*/

{

	DWORD rdata;
	

	EthCommand = (unsigned char *)ioPacketPage;

    v_pIOPRegs = (volatile S3C2440A_IOPORT_REG*)VirtualAlloc(0, sizeof(S3C2440A_IOPORT_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pIOPRegs==NULL)
	{
		DEBUGMSG (1,(TEXT("[CS8900] v_pIOPRegs is not allocated\n\r")));
		return FALSE;
	}
	if (!VirtualCopy((PVOID)v_pIOPRegs, (PVOID)(S3C2440A_BASE_REG_PA_IOPORT >> 8), sizeof(S3C2440A_IOPORT_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
		DEBUGMSG (1,(TEXT("[CS8900] v_pIOPRegs is not mapped\n\r")));
		goto cs8900_fail;
	}
	DEBUGMSG (1,(TEXT("[CS8900] v_pIOPRegs is mapped to %x\n\r"), v_pIOPRegs));

      v_pINTRegs = (volatile S3C2440A_INTR_REG*)VirtualAlloc(0, sizeof(S3C2440A_INTR_REG), MEM_RESERVE, PAGE_NOACCESS);
	if (v_pINTRegs== NULL)
	{
	    DEBUGMSG (1,(TEXT("[CS8900] v_pINTRegs is not allocated\n\r")));
	    goto cs8900_fail;
	}
	if (!VirtualCopy((PVOID)v_pINTRegs, (PVOID)(S3C2440A_BASE_REG_PA_INTR >> 8), sizeof(S3C2440A_INTR_REG), PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE))
	{
	   DEBUGMSG (1,(TEXT("[CS8900] v_pINTRegs is not mapped\n\r")));
	   goto cs8900_fail;
	}
       DEBUGMSG (1,(TEXT("[CS8900] v_pINTRegs is mapped to %x\n\r"), v_pINTRegs));
       
	v_pMEMRegs = (S3C2440A_MEMCTRL_REG*)VirtualAlloc(0,sizeof(S3C2440A_MEMCTRL_REG), MEM_RESERVE,PAGE_NOACCESS);
	if(v_pMEMRegs == NULL) 
	{
	    DEBUGMSG (1,(TEXT("[CS8900] v_pMEMRegs is not allocated\n\r")));
		goto cs8900_fail;
	}
	if(!VirtualCopy((PVOID)v_pMEMRegs,(PVOID)(S3C2440A_BASE_REG_PA_MEMCTRL >> 8),sizeof(S3C2440A_MEMCTRL_REG),PAGE_PHYSICAL | PAGE_READWRITE|PAGE_NOCACHE)) {
	    DEBUGMSG (1,(TEXT("[CS8900] v_pMEMRegs is not mapped\n\r")));
	    goto cs8900_fail;
	}    
	DEBUGMSG (1,(TEXT("[CS8900] v_pMEMRegs is mapped to %x\n\r"), v_pMEMRegs));


    // nGCS3=nUB/nLB(nSBHE),nWAIT,16-bit
    v_pMEMRegs->BWSCON = (v_pMEMRegs->BWSCON&~(0xf<<12))|(0xd<<12);	

    // BANK3 access timing
	v_pMEMRegs->BANKCON3=((CS8900_Tacs<<13)+(CS8900_Tcos<<11)+(CS8900_Tacc<<8)+(CS8900_Tcoh<<6)\
		+(CS8900_Tah<<4)+(CS8900_Tacp<<2)+(CS8900_PMC));

	rdata  = v_pIOPRegs->GPGCON;
	rdata &= ~(3 << 2);
	rdata |=  (2 << 2);
	v_pIOPRegs->GPGCON = rdata;			/* External Interrupt #9 Enable			*/

#if 1
	rdata  = v_pIOPRegs->EXTINT1;
	rdata &= ~(7 << 4);
	rdata |=  (4 << 4);
	v_pIOPRegs->EXTINT1 = rdata;			/* High Level Triggered Mode				*/
#else
	rdata  = v_pIOPRegs->EXTINT1;
	rdata &= ~(7 << 4);
	rdata |=  (1 << 4);
	v_pIOPRegs->EXTINT1 = rdata;			/* Rising Edge Detect Mode				*/
#endif

	DEBUGMSG(1, (TEXT("[CS8900] rGPGCON = %x\r\n"),	v_pIOPRegs->GPGCON));
	DEBUGMSG(1, (TEXT("[CS8900] rEXTINT1 = %x\r\n"), v_pIOPRegs->EXTINT1));


	if (CSInit() == FALSE)
	    {
	    	RETAILMSG(1, (TEXT("CS8900:CSInit failure!!\r\n")));
		return FALSE;
             } 
             
	return TRUE;

cs8900_fail:
    if (v_pIOPRegs) {
        VirtualFree((PVOID)v_pIOPRegs, 0, MEM_RELEASE);
    }
    if (v_pINTRegs) {
        VirtualFree((PVOID)v_pINTRegs, 0, MEM_RELEASE);
    }
    if (v_pMEMRegs) {
        VirtualFree((PVOID)v_pMEMRegs, 0, MEM_RELEASE);
    }
	return FALSE;
}

#pragma NDIS_PAGEABLE_FUNCTION(CS8900ReadEthernetAddress)

BOOLEAN CS8900ReadEthernetAddress(
    IN PCS8900_ADAPTER Adapter
)

/*++

Routine Description:

    Reads in the Ethernet address from the CS8900 Chip...

Arguments:

    Adapter - pointer to the adapter block.

Return Value:

    The address is stored in Adapter->PermanentAddress, and StationAddress if it
    is currently zero.

--*/

{
	Adapter->PermanentAddress[0] = 0x22;
	Adapter->PermanentAddress[1] = 0x33;
	Adapter->PermanentAddress[2] = 0x44;
	Adapter->PermanentAddress[3] = 0x55;
	Adapter->PermanentAddress[4] = 0x66;
	Adapter->PermanentAddress[5] = 0x0F;
      
	DEBUGMSG(1,
        (TEXT("CS8900: PermanentAddress [ %02x-%02x-%02x-%02x-%02x-%02x ]\r\n"),
            Adapter->PermanentAddress[0],
            Adapter->PermanentAddress[1],
            Adapter->PermanentAddress[2],
            Adapter->PermanentAddress[3],
            Adapter->PermanentAddress[4],
            Adapter->PermanentAddress[5]));

    //
    // Use the burned in address as the station address, unless the
    // registry specified an override value.
    //
    if ((Adapter->StationAddress[0] == 0x00) &&
        (Adapter->StationAddress[1] == 0x00) &&
        (Adapter->StationAddress[2] == 0x00) &&
        (Adapter->StationAddress[3] == 0x00) &&
        (Adapter->StationAddress[4] == 0x00) &&
        (Adapter->StationAddress[5] == 0x00)
	    )
    {
    	DEBUGMSG(1, (TEXT("CS8900: StationAddress Modified!...\r\n")));
        Adapter->StationAddress[0] = Adapter->PermanentAddress[0];
        Adapter->StationAddress[1] = Adapter->PermanentAddress[1];
        Adapter->StationAddress[2] = Adapter->PermanentAddress[2];
        Adapter->StationAddress[3] = Adapter->PermanentAddress[3];
        Adapter->StationAddress[4] = Adapter->PermanentAddress[4];
        Adapter->StationAddress[5] = Adapter->PermanentAddress[5];
    }

    return(TRUE);
}

unsigned short CS8900RequestTransmit
(
	UINT PacketLength
	)
{
	// Request that the transmit be started after all data has been copied
	writeIoPort(IO_TX_CMD, TX_CMD_START_ALL);
	writeIoPort(IO_TX_LENGTH, (unsigned short)PacketLength);

	// Return the BusStatus register which indicates success of the request
	return CS8900ReadRegister(PKTPG_BUS_ST);
}

void CS8900CopyTxFrame
(
	PCHAR pPacket,
	UINT PacketLength
    )
{
	UINT 	i;
	PWORD	pMsg;
	WORD	PacketType, PacketOper;

	pMsg = (PWORD)pPacket;

	PacketType = *(pMsg + 6);
	PacketOper = *(pMsg + 10);

	if (PacketType == 0x0608)
	{
		if (PacketOper == 0x0100)
			DEBUGMSG(1, (TEXT("[CS8900] Send ARP Request Packet\r\n")));
		else if (PacketOper == 0x0200)
			DEBUGMSG(1, (TEXT("[CS8900] Send ARP Response Packet\r\n")));
		else if (PacketOper == 0x0300)
			DEBUGMSG(1, (TEXT("[CS8900] Send RARP Request Packet\r\n")));
		else if (PacketOper == 0x0400)
			DEBUGMSG(1, (TEXT("[CS8900] Send RARP Response Packet\r\n")));
		else
			DEBUGMSG(1, (TEXT("[CS8900] Send Unknown ARP Packet\r\n")));
	}
	else if (PacketType == 0x0008)
		DEBUGMSG(1, (TEXT("[CS8900] Send IP Packet\r\n")));
	
	for (i = 0 ; i < (PacketLength + 1) / 2; i++)
	{
		writeIoPort(IO_RX_TX_DATA_0, *pMsg);
		pMsg++;
	}
}

