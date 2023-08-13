#ifndef XNET_TINT_H
#define XNET_TINY_H

#include <stdint.h>
// ����IP
#define XNET_CFG_NETIF_IP {192,168,137,2}
// �շ����ݰ�������С
#define XNET_CFG_PACKGE_MAX_SIZE 1516
// ARP���ʱʱ��
#define XARP_CFG_ENTRY_OK_TMO (5)
// ARP�������ʱʱ��
#define XARP_CFG_ENTRY_PENDING_TMO (1)
// ARP�����ʱ���Բ�ѯ����
#define XARP_CFG_MAX_RETRIES 4

#pragma pack(1)

// IP��ַ����
#define XNET_IPV4_ADDR_SIZE 4
// MAC��ַ����
#define XNET_MAC_ADDR_SIZE 6

/*
* ��̫������֡��ʽ��RFC894
*/
typedef struct _xether_hdr_t {
	// Ŀ��mac��ַ
	uint8_t dest[XNET_MAC_ADDR_SIZE];
	// Դmac��ַ
	uint8_t src[XNET_MAC_ADDR_SIZE];
	// Э��/����
	uint16_t protocol;
}xether_hdr_t;

// ��̫��
#define XARP_HW_ETHER 0x1
// ARP����
#define XARP_REQUEST 0x1
// ARP��Ӧ
#define XARP_REPLY 0x2

typedef struct _arp_packet_t {
	// Ӳ�����ͺ�Э�鳤��
	uint16_t hw_type, pro_type;
	// Ӳ����ַ�� + Э���ַ��
	uint8_t hw_len, pro_len;
	// ����/��Ӧ
	uint16_t opcode;
	// ���Ͱ�Ӳ����ַ
	uint8_t sender_mac[XNET_MAC_ADDR_SIZE];
	// ���Ͱ�Э���ַ
	uint8_t sender_ip[XNET_IPV4_ADDR_SIZE];
	// ���շ�Ӳ����ַ
	uint8_t target_mac[XNET_MAC_ADDR_SIZE];
	// ���շ�Э���ַ
	uint8_t target_ip[XNET_IPV4_ADDR_SIZE];
}xarp_packet_t;

typedef struct _xip_hdr_t {
	uint8_t hdr_len : 4;
	uint8_t version : 4;
	uint8_t tos;
	uint16_t total_len;
	uint16_t id;
	uint16_t flags_fragment;
	uint8_t ttl;
	uint8_t protocol;
	uint8_t hdr_checksum;
	uint8_t src_ip[XNET_IPV4_ADDR_SIZE];
	uint8_t dest_ip[XNET_IPV4_ADDR_SIZE];
}xip_hdr_t;


#pragma pack()

typedef enum _xnet_err_t {
	XNET_ERR_OK = 0,
	XNET_ERR_IO = -1,
	XNET_ERR_NONE = -2,
}xnet_err_t;

// �������ݽṹ
typedef struct _xnet_packet_t {
	// ������Ч���ݴ�С
	uint16_t size;
	// ����������ʼ��ַ
	uint8_t* data;
	// �����������
	uint8_t payload[XNET_CFG_PACKGE_MAX_SIZE];
}xnet_packet_t;
// ʱ�����ͣ����ص�ǰϵͳ���˶��ٸ�100ms
typedef uint32_t xnet_time_t;

const xnet_time_t xsys_get_time(void);
int xnet_check_tmo(xnet_time_t* time, uint32_t sec);

xnet_packet_t* xnet_alloc_for_send(uint16_t data_size);
xnet_packet_t* xnet_alloc_for_read(uint16_t data_size);

xnet_err_t xnet_driver_open(uint8_t* mac_addr);
xnet_err_t xnet_driver_send(xnet_packet_t* packet);
xnet_err_t xnet_driver_read(xnet_packet_t** packet);

typedef enum _xnet_protocol_t {
	// ARPЭ��
	XNET_PROTOCOL_ARP = 0x0806,
	// IPЭ��
	XNET_PROTOCOL_IP = 0x0800,
}xnet_protocol_t;

/*
* IP��ַ
*/
typedef union _xipaddr_t {
	// ��������ʽ�洢��ip
	uint8_t array[XNET_IPV4_ADDR_SIZE];
	// 32λ��ip��ַ
	uint32_t addr;
}xipaddr_t;

// ARP�������
#define XARP_ENTRY_FREE 0
// ARP��������ɹ�
#define XARP_ENTRY_OK 1
// ARP�������ڽ���
#define XARP_ENTRY_RESOLVING 2
// ARPɨ�����ڣ�1s
#define XARP_TIMER_PERIOD 1
/*
* ARP����
*/
typedef struct _xarp_entry_t {
	// ip��ַ
	xipaddr_t ipaddr;
	// mac��ַ
	uint8_t macaddr[XNET_MAC_ADDR_SIZE];
	// ״̬λ
	uint8_t state;
	// ��ǰ��ʱ
	uint16_t tmo;
	// ��ǰ���Դ���
	uint8_t retry_cnt;
}xarp_entry_t;
// arp���
void xarp_init(void);
int xarp_make_request(const xipaddr_t* ipaddr);
void xarp_in(xnet_packet_t* packet);
void xarp_poll(void);

// ip���
// IPV4
#define XNET_VERSION_IPV4 4
// ȱʡ��IP��TTLֵ
#define XNET_IP_DEFAULT_TTL 64
void xip_init(void);
void xip_in(xnet_packet_t* packet);
xnet_err_t xip_out(xnet_protocol_t protocol, xipaddr_t* dest_ip, xnet_packet_t* packet);

// ��ײ�
void xnet_init(void);
void xnet_poll(void);
#endif 