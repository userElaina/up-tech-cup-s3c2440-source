#ifndef _ZIGBEE_DEF
#define _ZIGBEE_DEF
//�����豸ΪCoord
#define I_AM_COORDINATOR        

//CoordĬ�ϵ�ַ
#define MAC_LONG_ADDR_BYTE0             (0x00)      
#define MAC_LONG_ADDR_BYTE1             (0x00)
#define MAC_LONG_ADDR_BYTE2             (0x00)
#define MAC_LONG_ADDR_BYTE3             (0x00)
#define MAC_LONG_ADDR_BYTE4             (0x00)      
#define MAC_LONG_ADDR_BYTE5             (0xa3)
#define MAC_LONG_ADDR_BYTE6             (0x04)
#define MAC_LONG_ADDR_BYTE7             (0x00)


//Э��ջ����
//MAC��
// FRAME����ش�����
#define MAC_MAX_FRAME_RETRIES           (4)

//FRAME���Ӧ��ʱ��
#define MAC_ACK_WAIT_DURATION           (TICK_SECOND*2)

//���������������Ӧ��ʱ��
#define MAC_RESPONSE_WAIT_TIME          (TICK_SECOND*2)

//EDɨ��ʱ��
#define MAC_ED_SCAN_PERIOD              (TICK_SECOND/4)

//beaconӦ��ʱ��
#define MAC_ACTIVE_SCAN_PERIOD          (TICK_SECOND*2)

//NWK��
//·��ʱ�����
//·�ɷ���ʱ��
#define NWK_ROUTE_DISCOVER_PERIOD   (TICK_SECOND*5)
//·������ID����ʱ��
#define NWK_ROUTE_REQUEST_ID_PERIOD TICK_SECOND

//APS��
//�˵�����
#define NUM_USER_ENDPOINTS	4

//����˵�(1-240)
#define alex_coord	0x02

#if (alex_coord < 1) || (alex_coord > 240)
	//#error Endpoint value out of range (1-240)
#endif
	
#endif
