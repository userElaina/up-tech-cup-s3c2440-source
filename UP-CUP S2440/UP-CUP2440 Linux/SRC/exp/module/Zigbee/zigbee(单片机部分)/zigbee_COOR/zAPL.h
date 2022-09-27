#ifndef _ZAPL_H_
#define _ZAPL_H_

#include "zigbee.h"
#ifndef Z_APS_h
#define Z_APS_h
#include "zAPS.h"
#endif
// These are states of the APL module.
typedef enum _SM_APL
{
    SM_APL_START,
    SM_APL_START_WAIT,
    SM_APL_READY,
    SM_APL_DISCOVERY,
    SM_APL_JOIN,
    SM_APL_JOIN_WAIT,
    SM_APL_REJOIN_WAIT
} SM_APL;

extern SM_APL smAPL;

//add by lyj_uptech@126.com
#define APLGetDestShortAddr() APSGetDestShortAddr()

//add by lyj_uptech@126.com
#define APLGetSrcShortAddr() APSGetSrcShortAddr()
//add by lyj_uptech@126.com
#define APLGetLongAddress(x) APSGetLongAddress(x)
//add by lyj_uptech@126.com
#define APLGetShortAddress(x) APSGetShortAddress(x)
/*********************************************************************
 * Function:        void APLInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Handle to matching node.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes individual Zigbee modules.
 *
 * Note:            None
 ********************************************************************/
void APLInit(void);

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
BOOL APLIsIdle();

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
void APLEnable(void);

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
	void APLDisable(void);
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
BOOL APLTask(void);


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
	void APLNetworkInit();

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
	BOOL APLIsNetworkInitComplete();

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
	void APLNetworkForm();

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
	void APLPermitAssociation();

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
	void APLDisableAssociation();

#elif defined(I_AM_END_DEVICE)

    /*********************************************************************
     * Macro:           void APLJoin(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Starts end device join process. After this call
     *                  repeatdly call ZAPLIsJoinComplete() to determine
     *                  if join process is complete.
     *
     * Note:            None
     ********************************************************************/
    #define APLJoin()                  (smAPL = SM_APL_START)

    /*********************************************************************
     * Macro:           BOOL APLIsJoinComplete(void);
     *
     * PreCondition:    APLJoin is called
     *
     * Input:           None
     *
     * Output:          TRUE if join process is complete
     *                      Must call GetLastZError() to determine
     *                      if join was successful.
     *                  FALSE otherwise
     *
     * Side Effects:    None
     *
     * Overview:        Performs join process and returns appropriate status.
     *
     * Note:            None
     ********************************************************************/
    BOOL APLIsJoinComplete(void);

    /*********************************************************************
     * Macro:           void APLRejoin(void)
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          None
     *
     * Side Effects:    None
     *
     * Overview:        Starts rejoin process.
     *                  Must call APLIsRejoinComplete() to find out if
     *                  rejoin is completed.
     *
     * Note:            None
     ********************************************************************/
    #define APLRejoin()                (smAPL = SM_APL_START)

    /*********************************************************************
     * Function:        BOOL APLIsRejoinComplete(void);
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          TRUE if rejoin is complete
     *                      Must call GetLastZError() to determine if
     *                      rejoin was successful.
     *                  FALSE otherwise
     *
     * Side Effects:    None
     *
     * Overview:        Starts rejoin process.
     *                  Must call ZAPLIsRejoinComplete() to find out if
     *                  rejoin is completed.
     *
     * Note:            None
     ********************************************************************/
    BOOL APLIsRejoinComplete(void);

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
	void APLLeave();

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
	BOOL APLIsLeaveComplete();

    /*********************************************************************
     * Function:        extern BOOL AppOkayToAssociate(void);
     *
     * PreCondition:    None
     *
     * Input:           None
     *
     * Output:          TRUE if application finds that is okay to
     *                  associate with recently discovered coordinator
     *                  FALSE otherwise
     *
     * Side Effects:    None
     *
     * Overview:        This is a callback to application.
     *                  This allows application to check newly discovered
     *                  coordinator and determine if it wants to
     *                  associate with it or not.
     *
     * Note:            None
     ********************************************************************/
    extern BOOL AppOkayToAssociate(void);

#endif

void APL_ISR();

EP_HANDLE APLOpenEP(BYTE ep);

void APLCloseEP();

void APLSetEP(EP_HANDLE ep); 

BOOL APLIsGetReady();

BOOL APLIsPutReady();

BYTE APLGet();

BYTE APLGetArray(BYTE *v, BYTE n);

BYTE APLGetClusterID();

void APLDiscardRx();

TRANS_ID APLSendMessage(MESSAGE_INFO *pMessageInfo);

BOOL APLIsAddressAssigned(void);

void APLPutLongAddress(LONG_ADDR *addr);

void APLPutShortAddress(SHORT_ADDR *addr);

void APLUpdateAddressInfo();

//#define APLGetPathInfo(x)  nwk_get_path_info(x)
#endif
