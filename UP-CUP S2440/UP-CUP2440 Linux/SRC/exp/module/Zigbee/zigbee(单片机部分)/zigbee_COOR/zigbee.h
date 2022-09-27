#ifndef _ZIGBEE_H_
#define _ZIGBEE_H_

#include "generic.h"

//only for debug
//#define ENABLE_DEBUG
//#define route_debug
//#define ROUTE_MONITOR

//Coord, RFD转换开关，打开为Coord,关闭为RFD
#define Coord

#ifdef Coord
	#include "zigbee_Coord.h"
#else
	#include "zigbee_RFD.h"
#endif

// Define Simple Constants
#define APS_MESSAGE_ACK_REQUESTED       0x04
#define APS_MESSAGE_DIRECT              0x01
#define APS_MESSAGE_INDIRECT            0x00
#define APS_MESSAGE_LONG_ADDRESS        0x02
#define APS_MESSAGE_NO_ACK              0x00
#define APS_MESSAGE_SHORT_ADDRESS       0x00

#define EP_HANDLE_INVALID               (0xff)

#define TRANS_ID_INVALID                (0x00)

typedef BYTE EP_HANDLE;

typedef struct _LONG_ADDR
{
    BYTE v[8];
} LONG_ADDR;

typedef union _SHORT_ADDR
{
    struct
    {
        BYTE LSB;
        BYTE MSB;
    } byte;
    WORD Val;
    BYTE v[2];
} SHORT_ADDR;

typedef SHORT_ADDR PAN_ADDR;

typedef enum _FRAME_TYPE
{
    KVP_FRAME,
    MSG_FRAME,
    OTHER_FRAME
} FRAME_TYPE;

typedef enum _TRANS_COMMAND_TYPE
{
    TRANS_SET           = 0x1 << 4,
    TRANS_EVENT         = 0x2 << 4,
    TRANS_GET_ACK       = 0x4 << 4,
    TRANS_SET_ACK       = 0x5 << 4,
    TRANS_EVENT_ACK     = 0x6 << 4,
    TRANS_GET_RESP      = 0x8 << 4,
    TRANS_SET_RESP      = 0x9 << 4,
    TRANS_EVENT_RESP    = 0xA << 4
} TRANS_COMMAND_TYPE;

typedef enum _TRANS_DATA_TYPE
{
    TRANS_NO_DATA       = 0x0,  // Attribute has no associated data
    TRANS_UINT8         = 0x1,  // 8-bit unsigned integer
    TRANS_INT8          = 0x2,  // 8-bit signed integer
    TRANS_UINT16        = 0x3,  // 16-bit unsigned integer
    TRANS_INT16         = 0x4,  // 16-bit signed integer
    TRANS_SEMI_PRECISE  = 0xB,  // 16-bit IEEE 754 floating point
    TRANS_ABS_TIME      = 0xC,  // 32-bit unsigned absolute time since midnight 1st January 2000 in seconds
    TRANS_REL_TIME      = 0xD,  // 32-bit unsigned relative time in milliseconds
    TRANS_ZSTRING       = 0xE,  // 1 byte length followed by array of characters encoded by a particular language/character set
    TRANS_OSTRING       = 0xF   // 1 byte length followed by an array of application defined octets
} TRANS_DATA_TYPE;

typedef enum _TRANS_FRAME_TYPE
{
    TRANS_FRAME_TYPE_KVP    = 0x01,
    TRANS_FRAME_TYPE_MSG    = 0x02
} TRANS_FRAME_TYPE;

typedef WORD PROFILE_ID;

typedef BYTE TRANS_ID;  

#define GENERATE_TRANS_ID               TRANS_ID_INVALID

#define MY_PROFILE_ID           0x0000

#define PICDEMZ_DEMO_CLUSTER_ID     (0)

typedef struct _MESSAGE_INFO
{
    union
    {
        struct
        {
            unsigned int bModeIsDirect  : 1; // Direct or Indirect addressing
            unsigned int bAddressIsLong : 1; // destAddress points to long/short address
            unsigned int bAckRequested  : 1; // Acknowledge requested at APS level
        } bits;
        BYTE Val;
    } flags;
    FRAME_TYPE          frameType;          // KVP, MSG, or other
    union {
        LONG_ADDR       longAddr;
        SHORT_ADDR      shortAddr;
    } destAddress;                          // Pointer to destination
    BYTE                destEP;
    PROFILE_ID          profileID;
    BYTE                clusterID;
    BYTE                srcEP;
    BYTE                dataLength;         // Length of data to transmit
    BYTE                *dataPointer;       // Pointer to message data
    // More items will be needed here when we have router support
    TRANS_ID            transactionID;
    TRANS_COMMAND_TYPE  command;        // GET/SET/etc. Command, if KVP
    TRANS_DATA_TYPE     dataType;       // Data Type, if KVP
    WORD_VAL            attribID;       // Attribute ID, if KVP
    BYTE                radius;         // the radius of the message
} MESSAGE_INFO;

// Define Function Macros
typedef enum _ZCODE
{
    ZCODE_NO_ERROR = 0,
    ZCODE_NO_ACK,
    ZCODE_CHANNEL_BUSY
} ZCODE;

#define GetLastZError()     (zErrorCode)
//应用程序不能访问此函数
#define SetZError(v)        (zErrorCode = v)

//发送数据包
extern MESSAGE_INFO                 currentMessageInfo;
//应用程序不能访问此变量
extern ZCODE                        zErrorCode;


#include "Console.h"
#include "MSPI.h"
#include "Tick.h"
#include "NVM.h"

#endif

