#ifndef _ZAPS_H_
#define _ZAPS_H_

#include "zigbee.h"
#include "zNWK.h"

#define NUM_DEFINED_ENDPOINTS   (1+NUM_USER_ENDPOINTS)

// APS frame control byte definition.
typedef union _APS_FRAME_CON
{
    struct
    {
        unsigned int type:2;
        unsigned int deliveryMode:2;
        unsigned int indirectAddressMode:1;
        unsigned int security:1;
        unsigned int ackRequested:1;
        unsigned int :1;
    } bits;
    BYTE Val;
} APS_FRAME_CON;

// APS frame header definition.
typedef struct _APS_HEADER
{
    APS_FRAME_CON frameCON;
    BYTE deliveryMode;
    BYTE destEP;
    BYTE clusterID;
    WORD_VAL profileID;
    BYTE srcEP;

    union
    {
        struct
        {
            unsigned int bIsInProcess:1;
        } bits;
        BYTE Val;
    } Flags;

    BYTE* payload;
} APS_HEADER;

typedef struct _EP
{
    HFRAME      hFrame;         // Handle to last sent frame.
    BYTE        ackClusterID;   // Cluster ID of acknowledged cluster

    union
    {
        struct
        {
            unsigned int bIsDirect:1;
            unsigned int bIsInUse:1;
            unsigned int bIsDataReady:1;
            unsigned int bIsOverflow:1;
            unsigned int bIsHeaderSent:1;
            unsigned int bIsTimedOut:1;
            unsigned int bACKReceived:1;
        } bits;
        BYTE Val;
    } flags;
} EP;

extern EP                           epList[];

extern EP                           *pCurrentEP;

extern APS_HEADER    apsCurrentFrame;

#define APS_ISR() NWK_ISR()

#define APSIsIdle() NWKIsIdle()

#define APSEnable() NWKEnable()

#define APSGet() NWKGet()

#define APSGetArray(v, n) NWKGetArray(v, n)

/*********************************************************************
 * Function:        void APSInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes APS variables.
 *
 * Note:            None
 ********************************************************************/
void APSInit(void);

/*********************************************************************
 * Macro:           void APSDisable(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Forgets about all EP transactions and prepares for
 *                  stack disable mode.
 *
 * Note:            None
 ********************************************************************/
void APSDisable(void);	

/*********************************************************************
 * Function:        BOOL APSTask(void)
 *
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if there is no outstanding states to execute
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This is a main APS task. It must be called
 *                  as many times as possible to keep APS module
 *                  running.
 *
 * Note:            None
 ********************************************************************/
BOOL APSTask(void);

/*********************************************************************
 * Function:        EP_HANDLE APSOpenEP(BYTE endPoint )
 *
 * PreCondition:    APSInit() is called
 *
 * Input:           endPoint    - endpoint number to enable
 *
 * Output:          handle to EP if there was one available EP
 *
 * Side Effects:    None
 *
 * Overview:        Enables the selected endpoint to receive data, and
 *                  returns a handle to the endpoint information.  Also
 *                  updates pCurrentEP to point to the newly enabled
 *                  endpoint.
 *
 * Note:            None
 ********************************************************************/
EP_HANDLE APSOpenEP(BYTE ep);

 /*********************************************************************
 * Macro:           void APSSetEP(EP_HANDLE h)
 *
 * PreCondition:    None
 *
 * Input:           h       - A valid handle to already opened EP.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets up current pointer.
 *
 * Note:            None
 ********************************************************************/
#define APSSetEP(h) (pCurrentEP = &epList[h])

/*********************************************************************
 * Macro:           void APSCloseEP(void)
 *
 * PreCondition:    APSSetEP(h) is called with valid handle
 *                  to desired EP.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Disables the current endpoint.
 *
 * Note:            None
 ********************************************************************/
#define APSCloseEP() (pCurrentEP->flags.bits.bIsInUse = FALSE)

/*********************************************************************
 * Function:        TRANS_ID APSDE_DATA_request( MESSAGE_INFO *pMessageInfo )
 *
 * PreCondition:    APSInit() is called
 *
 * Input:           pMessageInfo    - pointer to a message information
 *                                      structure
 *
 * Output:          If successful, the transaction ID of the message is
 *                  returned.  Otherwise, TRANS_ID_INVALID is returned.
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send a KVP, MSG or other frame.
 *
 * Note:            None
 ********************************************************************/
TRANS_ID APSDE_DATA_request( MESSAGE_INFO *pMessageInfo );

/*********************************************************************
 * Macro:           BOOL APSIsPutReady(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE if is it okay to transmit new APS frame.
 *                  FALSE, otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This is a macro abstraction to hide NWK macro.
 *
 * Note:            None
 ********************************************************************/
#define APSIsPutReady()             NWKIsPutReady()

/*********************************************************************
 * Macro:           BOOL APSIsGetReady(void)
 *
 * PreCondition:    APSSetEP() is called with valid EP handle.
 *
 * Input:           None
 *
 * Output:          TRUE, if current ep contains a data frame.
 *                  FALSE, otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Returns internal flag indicating if there is any
 *                  data pending for this ep.
 *
 * Note:            None
 ********************************************************************/
#define APSIsGetReady()             (pCurrentEP->flags.bits.bIsDataReady)

/*********************************************************************
 * Function:        void APSDiscard(void)
 *
 * PreCondition:    APSSetEP() is called with valid EP handle.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Clears internal flag indicating that this EP
 *                  is done processing the current frame.  If all
 *                  endpoints are done with the current frame, the
 *                  frame is released.
 *
 * Note:            None
 ********************************************************************/
void APSDiscardRx( void );

/*********************************************************************
 * Macro:           BYTE APSGetClusterID(void)
 *
 * PreCondition:    APSSetEP() is called with valid EP handle.
 *
 * Input:           None
 *
 * Output:          Cluster ID for current APS frame.
 *
 * Side Effects:    None
 *
 * Overview:        Returns received clusterID
 *
 * Note:            None
 ********************************************************************/
#define APSGetClusterID()           (apsCurrentFrame.clusterID)

#endif

