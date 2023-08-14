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
// ���֧�ֵ�UDP������
#define XUDP_CFG_MAX_UDP 10
// ���֧�ֵ�TCP������
#define XTCP_CFG_MAX_TCP 40
// TCP�շ���������С
#define XTCP_CFG_RTX_BUF_SIZE 2048
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
	// �ײ����ȣ�4�ֽ�Ϊ��λ
	uint8_t hdr_len : 4;
	// �汾��
	uint8_t version : 4;
	// ��������
	uint8_t tos;
	// �ܳ���
	uint16_t total_len;
	// ��ʶ��
	uint16_t id;
	// ��־��ֶ�
	uint16_t flags_fragment;
	// ���ʱ��
	uint8_t ttl;
	// �ϲ�Э��
	uint8_t protocol;
	// �ײ�У���
	uint8_t hdr_checksum;
	// ԴIP
	uint8_t src_ip[XNET_IPV4_ADDR_SIZE];
	// Ŀ��IP
	uint8_t dest_ip[XNET_IPV4_ADDR_SIZE];
}xip_hdr_t;

typedef struct _xicmp_hdr_t {
	// ����
	uint8_t type;
	// ����
	uint8_t code;
	// ICMP���ĵ�У���
	uint16_t checksum;
	// ��ʶ��
	uint16_t id;
	// ���
	uint16_t seq;
}xicmp_hdr_t;

typedef struct _xudp_hdr_t {
	// Դ�˿� + Ŀ��˿�
	uint16_t src_port, dest_port;
	// �������ݰ��ĳ���
	uint16_t total_len;
	//У���
	uint16_t checksum;
}xudp_hdr_t;

typedef struct _xtcp_hdr_t {
	// Դ�˿� + Ŀ��˿�
	uint16_t src_port, dest_port;
	// �Լ����͵����ݵ���ʼ���
	uint32_t seq;
	// ֪ͨ�Է��������ܵ���һ�ֽڵ����
	uint32_t ack;
	union {
		struct {
#define XTCP_FLAG_FIN (1<<0)
#define XTCP_FLAG_SYN (1<<1)
#define XTCP_FLAG_RST (1<<2)
#define XTCP_FLAG_ACK (1<<4)
			// ��־λ
			uint16_t flags : 6;
			// ����λ
			uint16_t reserved : 6;
			// �ײ����ȣ���4�ֽ�λΪ��λ
			uint16_t hdr_len : 4;
		};
		uint16_t all;
	}hdr_flags;
	// ���ڴ�С�����߶Է��Լ��ܽ��ܶ�������
	uint16_t window;
	// У���
	uint16_t checksum;
	// ����ָ��
	uint16_t urgent_ptr;
}xtcp_hdr_t;

#pragma pack()

typedef enum _xnet_err_t {
	XNET_ERR_OK = 0,
	XNET_ERR_IO = -1,
	XNET_ERR_NONE = -2,
	XNET_ERR_BINDED = -3,
	XNET_ERR_PARAM = -4,
	XNET_ERR_MEM = -5,
	XNET_ERR_STATE = -6,
	XNET_ERR_WIN_0 = -8,
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
void truncate_packet(xnet_packet_t* packet, uint16_t size);

xnet_err_t xnet_driver_open(uint8_t* mac_addr);
xnet_err_t xnet_driver_send(xnet_packet_t* packet);
xnet_err_t xnet_driver_read(xnet_packet_t** packet);

typedef enum _xnet_protocol_t {
	// ARPЭ��
	XNET_PROTOCOL_ARP = 0x0806,
	// IPЭ��
	XNET_PROTOCOL_IP = 0x0800,
	// ICMPЭ��
	XNET_PROTOCOL_ICMP = 1,
	// UDPЭ��
	XNET_PROTOCOL_UDP = 17,
	// TCPЭ��
	XNET_PROTOCOL_TCP = 6,
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
// ICMP���
// ��������
#define XICMP_CODE_ECHO_REQUEST 8
// ������Ӧ
#define XICMP_CODE_ECHO_REPLY 0
// Ŀ�겻�ɴ�
#define XICMP_TYPE_UNREACH 3
// �˿ڲ��ɴ�
#define XICMP_CODE_PORT_UNREACH 3
// Э�鲻�ɴ�
#define XICMP_CODE_PRO_UNREACH 2
void xicmp_init(void);
void xicmp_in(xipaddr_t *src_ip,xnet_packet_t* packet);
xnet_err_t xicmp_dest_unreach(uint8_t code, xip_hdr_t* ip_hdr);

typedef struct _xudp_t xudp_t;
typedef xnet_err_t(*xudp_handler_t)(xudp_t* udp, xipaddr_t* src_ip, uint16_t src_port, xnet_packet_t* packet);
struct _xudp_t {
	enum {
		// UDPδʹ��
		XUDP_STATE_FREE,
		// UDP��ʹ��
		XUDP_STATE_USED,
	}state;
	// ���ض˿�
	uint16_t local_port;
	// �¼�����ص�
	xudp_handler_t handler;
};
void xudp_init(void);
void xudp_in(xudp_t* udp, xipaddr_t* src_ip, xnet_packet_t* packet);
int xudp_out(xudp_t* udp, xipaddr_t* dest_ip, uint16_t dest_port, xnet_packet_t* packet);
xudp_t* xudp_open(xudp_handler_t handler);
void xudp_close(xudp_t* udp);
xudp_t* xudp_find(uint16_t port);
xnet_err_t xudp_bind(xudp_t* udp, uint16_t local_port);

// TCP���
typedef enum _tcp_state_t {
	XTCP_STATE_FREE,
	XTCP_STATE_CLOSED,
	XTCP_STATE_LISTEN,
	XTCP_STATE_SYNC_RECVD,
	XTCP_STATE_FIN_WAIT_1,
	XTCP_STATE_FIN_WAIT_2,
	XTCP_STATE_CLOSING,
	XTCP_STATE_TIMED_WAIT,
	XTCP_STATE_CLOSE_WAIT,
	XTCP_STATE_LAST_ACK,
}xtcp_state_t;

typedef enum _xtcp_conn_state_t {
	XTCP_CONN_CONNECTED,
	XTCP_CONN_DATA_RECV,
	XTCP_CONN_CLOSED,
}xtcp_conn_state_t;

typedef struct _xtcp_buf_t {
	// �ܵ������� + δ���͵�������
	uint16_t data_count, unacked_count;
	// ��ʼ����������һ������λ�� 
	uint16_t front, tail, next;
	// ���ݻ���ռ�
	uint8_t data[XTCP_CFG_RTX_BUF_SIZE];
}xtcp_buf_t;

#define XTCP_KIND_END 0
#define XTCP_KIND_MSS 2
#define XTCP_MSS_DEFAULT 1460
typedef struct _xtcp_t xtcp_t;
typedef xnet_err_t(*xtcp_handler_t)(xtcp_t*tcp,xtcp_conn_state_t event);
struct _xtcp_t {
	// ״̬
	xtcp_state_t state;
	// ���ض˿� + Դ�˿�
	uint16_t local_port, remote_port;
	// ԴIP
	xipaddr_t remote_ip;
	// δȷ������ʼ��ţ���һ�������
	uint32_t unack_seq, next_seq;
	// �����Է������İ����
	uint32_t ack;
	//�Է���mss������ѡ����
	uint16_t remote_mss;
	// �Է��Ĵ��ڴ�С
	uint16_t remote_win;
	// �¼�������
	xtcp_handler_t handler;
	// �շ�������
	xtcp_buf_t rx_buf, tx_buf;
};

void xtcp_init(void);
void xtcp_in(xipaddr_t* remote_ip, xnet_packet_t* packet);
xtcp_t* xtcp_open(xtcp_handler_t handler);
xnet_err_t xtcp_bind(xtcp_t* tcp, uint16_t local_port);
xnet_err_t xtcp_listen(xtcp_t* tcp);
xnet_err_t xtcp_close(xtcp_t* tcp);
uint16_t xtcp_read(xtcp_t* tcp, uint8_t* data, uint16_t size);
int xtcp_write(xtcp_t* tcp, uint8_t* data, uint16_t size);
// ��ײ�
void xnet_init(void);
void xnet_poll(void);
#endif 