#ifndef XNET_TINT_H
#define XNET_TINY_H

#include <stdint.h>
// 本地IP
#define XNET_CFG_NETIF_IP {192,168,137,2}
// 收发数据包的最大大小
#define XNET_CFG_PACKGE_MAX_SIZE 1516
// ARP表项超时时间
#define XARP_CFG_ENTRY_OK_TMO (5)
// ARP表项挂起超时时间
#define XARP_CFG_ENTRY_PENDING_TMO (1)
// ARP表挂起时重试查询次数
#define XARP_CFG_MAX_RETRIES 4
// 最大支持的UDP连接数
#define XUDP_CFG_MAX_UDP 10

#pragma pack(1)

// IP地址长度
#define XNET_IPV4_ADDR_SIZE 4
// MAC地址长度
#define XNET_MAC_ADDR_SIZE 6

/*
* 以太网数据帧格式：RFC894
*/
typedef struct _xether_hdr_t {
	// 目标mac地址
	uint8_t dest[XNET_MAC_ADDR_SIZE];
	// 源mac地址
	uint8_t src[XNET_MAC_ADDR_SIZE];
	// 协议/长度
	uint16_t protocol;
}xether_hdr_t;

// 以太网
#define XARP_HW_ETHER 0x1
// ARP请求
#define XARP_REQUEST 0x1
// ARP响应
#define XARP_REPLY 0x2

typedef struct _arp_packet_t {
	// 硬件类型和协议长度
	uint16_t hw_type, pro_type;
	// 硬件地址长 + 协议地址长
	uint8_t hw_len, pro_len;
	// 请求/响应
	uint16_t opcode;
	// 发送包硬件地址
	uint8_t sender_mac[XNET_MAC_ADDR_SIZE];
	// 发送包协议地址
	uint8_t sender_ip[XNET_IPV4_ADDR_SIZE];
	// 接收方硬件地址
	uint8_t target_mac[XNET_MAC_ADDR_SIZE];
	// 接收方协议地址
	uint8_t target_ip[XNET_IPV4_ADDR_SIZE];
}xarp_packet_t;

typedef struct _xip_hdr_t {
	// 首部长度，4字节为单位
	uint8_t hdr_len : 4;
	// 版本号
	uint8_t version : 4;
	// 服务类型
	uint8_t tos;
	// 总长度
	uint16_t total_len;
	// 标识符
	uint16_t id;
	// 标志与分段
	uint16_t flags_fragment;
	// 存活时间
	uint8_t ttl;
	// 上层协议
	uint8_t protocol;
	// 首部校验和
	uint8_t hdr_checksum;
	// 源IP
	uint8_t src_ip[XNET_IPV4_ADDR_SIZE];
	// 目标IP
	uint8_t dest_ip[XNET_IPV4_ADDR_SIZE];
}xip_hdr_t;

typedef struct _xicmp_hdr_t {
	// 类型
	uint8_t type;
	// 代码
	uint8_t code;
	// ICMP报文的校验和
	uint16_t checksum;
	// 标识符
	uint16_t id;
	// 序号
	uint16_t seq;
}xicmp_hdr_t;

typedef struct _xudp_hdr_t {
	// 源端口 + 目标端口
	uint16_t src_port, dest_port;
	// 整个数据包的长度
	uint16_t total_len;
	//校验和
	uint16_t checksum;
}xudp_hdr_t;

#pragma pack()

typedef enum _xnet_err_t {
	XNET_ERR_OK = 0,
	XNET_ERR_IO = -1,
	XNET_ERR_NONE = -2,
	XNET_ERR_BINDED = -3,
	XNET_ERR_PARAM = -4,
	XNET_ERR_MEM = -5,
}xnet_err_t;

// 网络数据结构
typedef struct _xnet_packet_t {
	// 包中有效数据大小
	uint16_t size;
	// 包中数据起始地址
	uint8_t* data;
	// 最大负载数据量
	uint8_t payload[XNET_CFG_PACKGE_MAX_SIZE];
}xnet_packet_t;
// 时间类型，返回当前系统跑了多少个100ms
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
	// ARP协议
	XNET_PROTOCOL_ARP = 0x0806,
	// IP协议
	XNET_PROTOCOL_IP = 0x0800,
	// ICMP协议
	XNET_PROTOCOL_ICMP = 1,
	// UDP协议
	XNET_PROTOCOL_UDP = 17,
}xnet_protocol_t;

/*
* IP地址
*/
typedef union _xipaddr_t {
	// 以数据形式存储的ip
	uint8_t array[XNET_IPV4_ADDR_SIZE];
	// 32位的ip地址
	uint32_t addr;
}xipaddr_t;

// ARP表项空闲
#define XARP_ENTRY_FREE 0
// ARP表项解析成功
#define XARP_ENTRY_OK 1
// ARP表项正在解析
#define XARP_ENTRY_RESOLVING 2
// ARP扫描周期，1s
#define XARP_TIMER_PERIOD 1
/*
* ARP表项
*/
typedef struct _xarp_entry_t {
	// ip地址
	xipaddr_t ipaddr;
	// mac地址
	uint8_t macaddr[XNET_MAC_ADDR_SIZE];
	// 状态位
	uint8_t state;
	// 当前超时
	uint16_t tmo;
	// 当前重试次数
	uint8_t retry_cnt;
}xarp_entry_t;
// arp相关
void xarp_init(void);
int xarp_make_request(const xipaddr_t* ipaddr);
void xarp_in(xnet_packet_t* packet);
void xarp_poll(void);

// ip相关
// IPV4
#define XNET_VERSION_IPV4 4
// 缺省的IP包TTL值
#define XNET_IP_DEFAULT_TTL 64
void xip_init(void);
void xip_in(xnet_packet_t* packet);
xnet_err_t xip_out(xnet_protocol_t protocol, xipaddr_t* dest_ip, xnet_packet_t* packet);
// ICMP相关
// 回显请求
#define XICMP_CODE_ECHO_REQUEST 8
// 回显响应
#define XICMP_CODE_ECHO_REPLY 0
// 目标不可达
#define XICMP_TYPE_UNREACH 3
// 端口不可达
#define XICMP_CODE_PORT_UNREACH 3
// 协议不可达
#define XICMP_CODE_PRO_UNREACH 2
void xicmp_init(void);
void xicmp_in(xipaddr_t *src_ip,xnet_packet_t* packet);
xnet_err_t xicmp_dest_unreach(uint8_t code, xip_hdr_t* ip_hdr);

typedef struct _xudp_t xudp_t;
typedef xnet_err_t(*xudp_handler_t)(xudp_t* udp, xipaddr_t* src_ip, uint16_t src_port, xnet_packet_t* packet);
struct _xudp_t {
	enum {
		// UDP未使用
		XUDP_STATE_FREE,
		// UDP已使用
		XUDP_STATE_USED,
	}state;
	// 本地端口
	uint16_t local_port;
	// 事件处理回调
	xudp_handler_t handler;
};
void xudp_init(void);
void xudp_in(xudp_t* udp, xipaddr_t* src_ip, xnet_packet_t* packet);
int xudp_out(xudp_t* udp, xipaddr_t* dest_ip, uint16_t dest_port, xnet_packet_t* packet);
xudp_t* xudp_open(xudp_handler_t handler);
void xudp_close(xudp_t* udp);
xudp_t* xudp_find(uint16_t port);
xnet_err_t xudp_bind(xudp_t* udp, uint16_t local_port);

// 最底层
void xnet_init(void);
void xnet_poll(void);
#endif 