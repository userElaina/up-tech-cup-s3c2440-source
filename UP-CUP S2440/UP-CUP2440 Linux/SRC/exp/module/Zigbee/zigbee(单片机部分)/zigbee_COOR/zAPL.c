#include "zAPL.h"
#ifndef Z_APS_h
#define Z_APS_h
#include "zAPS.h"
#endif
#include "zNWK.h"

SM_APL smAPL;

// Stack error code - used by all modules.
ZCODE zErrorCode;

/*********************************************************************
 * Function:        void APLInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the individual Zigbee stack modules.
 *
 * Note:            None
 ********************************************************************/
void APLInit(void)
{
    APSInit();
    // Initialize state machine
    smAPL = SM_APL_START;
}

/*********************************************************************
 * Macro:           BOOL APLIsIdle(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if APL is idle and may be powered down
 *                  FALSE if otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Checks to see any transmission is in progress.
 *                  If there is no outstanding transmission, APL
 *                  is said to be idle.
 *
 * Note:            Used in conjunction with APLDisable()
 ********************************************************************/
BOOL APLIsIdle()
{
	return APSIsIdle();
}

/*********************************************************************
 * Macro:           void APLEnable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Enables stack for normal operation.
 *
 * Note:            None
 ********************************************************************/
void APLEnable(void)
{
	APSEnable();
}

/*********************************************************************
 * Macro:           void APLDisable(void)
 *
 * PreCondition:    APLIsIdle() = TRUE
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Disables stack for potential power save mode.
 *
 * Note:            Available to end device only.
 *                  Coordinator is not expected to power down save.
 *                  It must be prepared to serve any of the node in
 *                  the network.
 ********************************************************************/
#if defined(I_AM_END_DEVICE)     
	void APLDisable(void)
	{
		APSDisable();
	}
#endif

#if defined(I_AM_COORDINATOR)
    /*********************************************************************
     * Macro:           void ZAPLNetworkInit(void)
     *
     * PreCondition:    ZAPLInit() is called.
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Starts the coordiantor network initialization
     *                  must call APLIsNetworkInitComplete() to
     *                  determine if initialization is complete.
     *
     * Note:            None
     ********************************************************************/           
	void APLNetworkInit()
	{
		 NWKCoordInit();
	}

    /*********************************************************************
     * Macro:           BOOL APLIsNetworkInitComplete(void)
     *
     * PreCondition:    ZAPLNetworkInit() is called
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Determines if network init is complete
     *
     * Note:            None
     ********************************************************************/
	BOOL APLIsNetworkInitComplete()
	{
		return NWKIsCoordInitComplete();
	}

    /*********************************************************************
     * Macro:           void APLNetworkForm(void)
     *
     * PreCondition:    APLNetworkInit() is called
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Determines if network init is complete
     *
     * Note:            None
     ********************************************************************/           
	void APLNetworkForm()
	{
		NWKForm();
	}

    /*********************************************************************
     * Macro:           void APLPermitAssociation(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Instructs NWK/MAC layer to accpet new associations
     *
     * Note:            None
     ********************************************************************/  
	void APLPermitAssociation()
	{
		NWKPermitJoining();
	}

    /*********************************************************************
     * Macro:           void APLDisableAssociation(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Stops accpeting new associations
     *
     * Note:            None
     ********************************************************************/    
	void APLDisableAssociation()
	{
		NWKDisableJoining();
	}

#endif

#if defined(I_AM_END_DEVICE)
	/*********************************************************************
	 * Function:        BOOL APLIsRejoinComplete(void)
	 *
	 * PreCondition:    APLStartRejoin() is called
	 *
	 * Input:           None
	 *
	 * Output:          TRUE if rejoin process is complete
	 *                  FALSE otherwise
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Performs rejoin as per IEEE spec.
	 *
	 * Note:            Available to end device only.
	 ********************************************************************/
	BOOL APLIsRejoinComplete(void)
	{
		switch(smAPL)
		{
		case SM_APL_START:
			// First try to see what is on the network
			NWKStartDiscovery();
			smAPL = SM_APL_START_WAIT;
			break;

		case SM_APL_START_WAIT:
			// Wait until it is complete.
			if ( NWKIsDiscoveryComplete() )
			{
				// Once complete, start the rejoin.
				if ( NWKIsDiscovered() )
				{
					if( AppOkayToAssociate() )
					{
						NWKStartRejoin();
						smAPL = SM_APL_REJOIN_WAIT;
					}
				}

				// If there was an error, return and let application decide what to do.
				if ( GetLastZError() != ZCODE_NO_ERROR )
					return TRUE;
			}
			break;

		case SM_APL_REJOIN_WAIT:
			// Wait for rejoin to complete.
			if ( NWKIsRejoinComplete() )
			{
				// Did we join to any network?
				if ( NWKIsJoined() )
					return TRUE;

				// Else keep discovering it.
				else
					smAPL= SM_APL_START_WAIT;
			}
			break;
		}

		return FALSE;
	}

	/*********************************************************************
	 * Function:        BOOL APLIsJoinComplete(void)
	 *
	 * PreCondition:    APLStartJoin() is called
	 *
	 * Input:           None
	 *
	 * Output:          TRUE if join process is complete
	 *                  FALSE otherwise
	 *
	 * Side Effects:    None
	 *
	 * Overview:        Performs new join as per IEEE spec.
	 *
	 * Note:            Available to end device only.
	 ********************************************************************/
	BOOL APLIsJoinComplete(void)
	{
		switch(smAPL)
		{
		case SM_APL_START:
			// Discovery networks
			NWKStartDiscovery();
			smAPL = SM_APL_START_WAIT;
			break;

		case SM_APL_START_WAIT:
			// Wait until we discovery any
			if ( NWKIsDiscoveryComplete() )
			{
				if ( NWKIsDiscovered() )
				{
					// Now that we have discovered a network, ask application
					// to see if it is okay to associate with this network.
					if( AppOkayToAssociate() )
					{
						// Application is okay with this network
						// Accept this network.
						NWKAcceptCurrentPAN();

						// Start the join process.
						smAPL = SM_APL_JOIN;
					}
					// else next channel, next network will be tried.
				}
		
				else
				{
					if ( GetLastZError() != ZCODE_NO_ERROR )
						// There was an error while trying to discover
						// a network. Finish this function and let
						// application handle it.
						return TRUE;
				}
			}
			break;

		case SM_APL_JOIN:
			NWKStartJoin();
			smAPL = SM_APL_JOIN_WAIT;
			break;

		case SM_APL_JOIN_WAIT:
			// Wait till we join.
			if ( NWKIsJoinComplete() )
			{
				// Did we join?
				if ( NWKIsJoined() )
					return TRUE;

				else
					smAPL = SM_APL_START;
			}
			break;
		}

		return FALSE;
	}

    /*********************************************************************
     * Macro:           void APLLeave(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Initiates network leave procedure
     *
     * Note:            None
     ********************************************************************/               
	void APLLeave()
	{
		NWKLeave();
	}

    /*********************************************************************
     * Macro:           BOOL APLIsLeaveComplete(void);
     *
     * PreCondition:    APLJoin is called
     *
     * Input:           None
     *
     * Output:          TRUE if leave process is complete
     *                      Must call GetLastZError() to determine
     *                      if join was successful.
     *                  FALSE otherwise
     *
     * Side Effects:    None
     *
     * Overview:        Performs leave process and returns appropriate status.
     *
     * Note:            None
     ********************************************************************/         
	BOOL APLIsLeaveComplete()
	{
		return NWKIsLeaveComplete();
	}

#endif

/*********************************************************************
 * Function:        BOOL APLTask(void)
 *
 * PreCondition:    ZAPLInit() is called.
 *
 * Input:           None
 *
 * Output:          Always TRUE in this version.
 *
 * Side Effects:    None
 *
 * Overview:        Calls all zigbee tasks in right order.
 *
 * Note:            None
 ********************************************************************/
BOOL APLTask(void)
{
//	ConsolePutString(__FUNCTION__);
//	ConsolePutString("\r\n");
    APSTask();
    return TRUE;
}


void APL_ISR()
{
	APS_ISR();
}


EP_HANDLE APLOpenEP(BYTE ep)
{
	return APSOpenEP(ep);
}

void APLCloseEP()
{
	APSCloseEP();
}

void APLSetEP(EP_HANDLE ep)
{
	APSSetEP(ep);
}

BOOL APLIsGetReady()
{
	return APSIsGetReady();
}

BOOL APLIsPutReady()
{
	return APSIsPutReady();
}

BYTE APLGet()
{
	return APSGet();
}

BYTE APLGetArray(BYTE *v, BYTE n)
{
	return APSGetArray(v,n);
}

BYTE APLGetClusterID()
{
	return APSGetClusterID();
}

void APLDiscardRx()
{
	APSDiscardRx();
}

TRANS_ID APLSendMessage(MESSAGE_INFO *pMessageInfo)
{
	return APSDE_DATA_request(pMessageInfo);
}

void APLUpdateAddressInfo()
{
	NWKUpdateAddressInfo();
}

void APLPutLongAddress(LONG_ADDR *addr)
{
	NWKPutLongAddress(addr);
}

void APLPutShortAddress(SHORT_ADDR *addr)
{
	NWKPutShortAddress(addr);
}

BOOL APLIsAddressAssigned(void)
{
	return NWKIsAddressAssigned();
}


