#ifndef _ZIGBEE_DEF
#define _ZIGBEE_DEF
//定义设备为Coord
#define I_AM_COORDINATOR        

//Coord默认地址
#define MAC_LONG_ADDR_BYTE0             (0x00)      
#define MAC_LONG_ADDR_BYTE1             (0x00)
#define MAC_LONG_ADDR_BYTE2             (0x00)
#define MAC_LONG_ADDR_BYTE3             (0x00)
#define MAC_LONG_ADDR_BYTE4             (0x00)      
#define MAC_LONG_ADDR_BYTE5             (0xa3)
#define MAC_LONG_ADDR_BYTE6             (0x04)
#define MAC_LONG_ADDR_BYTE7             (0x00)


//协议栈参数
//MAC层
// FRAME最大重传次数
#define MAC_MAX_FRAME_RETRIES           (4)

//FRAME最大应答时间
#define MAC_ACK_WAIT_DURATION           (TICK_SECOND*2)

//加入网络请求最大应答时间
#define MAC_RESPONSE_WAIT_TIME          (TICK_SECOND*2)

//ED扫描时间
#define MAC_ED_SCAN_PERIOD              (TICK_SECOND/4)

//beacon应答时间
#define MAC_ACTIVE_SCAN_PERIOD          (TICK_SECOND*2)

//NWK层
//路由时间参数
//路由发现时间
#define NWK_ROUTE_DISCOVER_PERIOD   (TICK_SECOND*5)
//路有请求ID生存时间
#define NWK_ROUTE_REQUEST_ID_PERIOD TICK_SECOND

//APS层
//端点数量
#define NUM_USER_ENDPOINTS	4

//定义端点(1-240)
#define alex_coord	0x02

#if (alex_coord < 1) || (alex_coord > 240)
	//#error Endpoint value out of range (1-240)
#endif
	
#endif
