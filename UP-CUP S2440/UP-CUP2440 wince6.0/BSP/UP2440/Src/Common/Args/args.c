#include <bsp.h>

VOID OALArgsInit(BSP_ARGS* pArgs)
{
	OALMSG(1,(TEXT("+OALArgsInit()\r\n")));
    // Check the BSP Args area
    //
    if (pArgs->header.signature  != OAL_ARGS_SIGNATURE ||
        pArgs->header.oalVersion != OAL_ARGS_VERSION   ||
        pArgs->header.bspVersion != BSP_ARGS_VERSION)
    {
      S3C2440A_IOPORT_REG *v_IOPRegs;
      DWORD count, code, j;
	  UCHAR d;

      // Zero out the current contents of the structure
      memset(pArgs, 0, sizeof(BSP_ARGS));

      // Setup header
      pArgs->header.signature = OAL_ARGS_SIGNATURE;
      pArgs->header.oalVersion = OAL_ARGS_VERSION;
      pArgs->header.bspVersion = BSP_ARGS_VERSION;

      //Set-up dummy uuid for SMDK2442. Actually, uuid can be acquired from NAND id. 
      pArgs->uuid[0] = (UCHAR)0x00;
      pArgs->uuid[1] = (UCHAR)0x11;
      pArgs->uuid[2] = (UCHAR)0x22;
      pArgs->uuid[3] = (UCHAR)0x33;
      pArgs->uuid[4] = (UCHAR)0x44;
      pArgs->uuid[5] = (UCHAR)0x55;
      pArgs->uuid[6] = (UCHAR)0x66; 
      pArgs->uuid[7] = (UCHAR)0x77;  
      pArgs->uuid[8] = (UCHAR)0x88;
      pArgs->uuid[9] = (UCHAR)0x99;
      pArgs->uuid[10] = (UCHAR)0xaa;
      pArgs->uuid[11] = (UCHAR)0xbb;
      pArgs->uuid[12] = (UCHAR)0xcc;
      pArgs->uuid[13] = (UCHAR)0xdd;
      pArgs->uuid[14] = (UCHAR)0xee;
      pArgs->uuid[15] = (UCHAR)0xff;
      
      //Set-up devid for SMDK2442.
	  count = sizeof(BSP_DEVICE_PREFIX) - 1;	// BSP_DEVICE_PREFIX= "SMDK2442" defined in bsp_cfg.h
	  if (count > sizeof(pArgs->deviceId)/2) count = sizeof(pArgs->deviceId)/2;
	  memcpy(pArgs->deviceId, BSP_DEVICE_PREFIX, count);
	  
 	 // Create unique part of name from SoC ID
      v_IOPRegs = OALPAtoUA(S3C2440A_BASE_REG_PA_IOPORT);
	  code = v_IOPRegs->GSTATUS1;	
		
	  // Convert it to hexa number	
	  for (j = 28; j >= 0 && count < sizeof(pArgs->deviceId); j -= 4) {
		d = (UCHAR)((code >> j) & 0xF);
		pArgs->deviceId[count++] = d < 10 ? '0' + d : 'A' + d - 10;
		}
				
	  // End string
	   while (count < sizeof(pArgs->deviceId)) pArgs->deviceId[count++] = '\0';

     // Can Add code for cleanboot, hiveclean, formatpartion
     
     OALMSG(1,(TEXT("Arguments area is initialized\r\n")));
    }
    else
    {
    	RETAILMSG(1,(TEXT("Arguments area has some values\r\n")));
    }
    OALMSG(1,(TEXT("-OALArgsInit()\r\n")));

    return;
}


//------------------------------------------------------------------------------
//
//  Function:  OALArgsQuery
//
//  This function is called from other OAL modules to return boot arguments.
//  Boot arguments are typically placed in fixed memory location and they are
//  filled by boot loader. In case that boot arguments can't be located
//  the function should return NULL. The OAL module then must use default
//  values.
//


VOID* OALArgsQuery(UINT32 type)
{
    VOID *pData = NULL;
    BSP_ARGS *pArgs;


    OALMSG(OAL_ARGS&&OAL_FUNC, (L"+OALArgsQuery(%d)\r\n", type));


    // Get pointer to expected boot args location
    pArgs = (BSP_ARGS*)IMAGE_SHARE_ARGS_UA_START;

    // Check if there is expected signature
    if (
        pArgs->header.signature  != OAL_ARGS_SIGNATURE ||
        pArgs->header.oalVersion != OAL_ARGS_VERSION   ||
        pArgs->header.bspVersion != BSP_ARGS_VERSION
    ) goto cleanUp;

    // Depending on required args
    switch (type) {
    case OAL_ARGS_QUERY_DEVID:
        pData = &pArgs->deviceId;
        break;
    case OAL_ARGS_QUERY_UUID:
        pData = &pArgs->uuid;
        break;        
    case OAL_ARGS_QUERY_KITL:
        pData = &pArgs->kitl;
        break;
    case BSP_ARGS_QUERY_HIVECLEAN:
        pData = &pArgs->bHiveCleanFlag;
        break;

    case BSP_ARGS_QUERY_CLEANBOOT:
        pData = &pArgs->bCleanBootFlag;
        break;

    case BSP_ARGS_QUERY_FORMATPART:
        pData = &pArgs->bFormatPartFlag;
        break;
    
    default:
        break;            
    }

cleanUp:
    OALMSG(OAL_ARGS&&OAL_FUNC, (L"-OALArgsQuery(pData = 0x%08x)\r\n", pData));
    return pData;
}

//------------------------------------------------------------------------------

