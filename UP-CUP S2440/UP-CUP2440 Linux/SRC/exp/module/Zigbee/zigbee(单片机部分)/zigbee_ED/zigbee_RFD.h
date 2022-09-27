#ifndef _ZIGBEE_DEF_H
#define _ZIGBEE_DEF_H
//�����豸ΪRFD
#define I_AM_END_DEVICE

//RFDĬ�ϵ�ַ
#define MAC_LONG_ADDR_BYTE0             (0x01)      
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
#define MAC_ACK_WAIT_DURATION           (TICK_SECOND*2) //modify by lyj *2 - > *4

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
//����˵㣨1��240��
#define alex_ed	0x02

#if (alex_ed < 1) || (alex_ed > 240)
	//#error Endpoint value out of range (1-240)
#endif

#endif
