/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 2001  Microsoft Corporation

Function:		CountNumberOfOnes()

Description:	Counts the number of bits that are "1" in a byte.

Returns:		Number of bits that are "1".
-------------------------------------------------------------------------------*/
#include <windows.h>

UCHAR CountNumberOfOnes(UCHAR num)
{
	UCHAR count = 0;

	while(num)
	{
		num=num&(num-1);
		count++;
	}

	return count;
}


#define DATA_BUFF_LEN			512
#define ECC_BUFF_LEN			3					// # of bytes in ECC


#define	NO_DATA_ERROR			0
#define ECC_ERROR				1
#define CORRECTABLE_ERROR		12					// Half of the ECC bits are 1

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:		ECC_CorrectData()

Description:	Corrects any errors (if possible) in the specified data.

Notes:			This implemention uses 3 bytes of ECC info for every 512 bytes
				of data.  Furthermore, a only single-bit error can be corrected
				for every 512 bytes of data.

				This code is based on the ECC algorithm publicly available on
				Samsung's FLASH media website.

Returns:		Boolean indicating if the data was corrected.
-------------------------------------------------------------------------------*/
BOOL ECC_CorrectData(LPBYTE pData, LPBYTE pExistingECC, LPBYTE pNewECC)
{
	DWORD i, numOnes, byteLocation, bitLocation;
	BYTE xorECC[ECC_BUFF_LEN];

	//----- 1. Check the parameters -----
	if((pData == NULL) || (pExistingECC == NULL) || (pNewECC == NULL))
	{
		return FALSE;
	}

	//----- 2. First, determine if this is a single-bit, correctable, error -----
	//		   NOTE: To answer this question, the two ECC values are XOR'd 
	//				 together and the total # of 1 bits is counted, which 
	//				 then tell us if we can correct the erroneous single-bit 
	//				 transition in the data.
	for(i=0; i<ECC_BUFF_LEN; i++)
	{
		xorECC[i] = pExistingECC[i] ^ pNewECC[i];
	}

	numOnes = 0;
	for(i=0; i<ECC_BUFF_LEN; i++)		
	{
		numOnes += CountNumberOfOnes(xorECC[i]);
	}

	switch(numOnes)
	{
		case NO_DATA_ERROR:					// Data doesn't contain an error
		return TRUE;

		case ECC_ERROR:						// Existing ECC value has gone bad!
		return FALSE;

		case CORRECTABLE_ERROR:				// Single-bit error
		break;

		default:							// More than a single-bit error
		return FALSE;
	}
		
	//----- 3. Compute the location of the single-bit error -----
	byteLocation = ( ((xorECC[2]&0x02)<<7) |
					 ((xorECC[1]&0x80))    | ((xorECC[1]&0x20)<<1) | 
			         ((xorECC[1]&0x08)<<2) | ((xorECC[1]&0x02)<<3) |
					 ((xorECC[0]&0x80)>>4) | ((xorECC[0]&0x20)>>3) | 
					 ((xorECC[0]&0x08)>>2) | ((xorECC[0]&0x02)>>1) );

	bitLocation  = (((xorECC[2]&0x80)>>5) | ((xorECC[2]&0x20)>>4) | ((xorECC[2]&0x08)>>3) );

	//----- 4. Correct the single-bit error (set the bit to its complementary value) -----
	if(pData[byteLocation] & (0x01 << bitLocation))
	{
		pData[byteLocation] &= ~(0x01 << bitLocation);		// 0->1 error, set bit to 0
	}else
	{
		pData[byteLocation] |= (0x01 <<  bitLocation);		// 1->0 error, set bit to 1
	}

	return TRUE;
}
