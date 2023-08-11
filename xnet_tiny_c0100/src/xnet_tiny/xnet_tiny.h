#ifndef XNET_TINT_H
#define XNET_TINY_H

#include <stdint.h>
// 收发数据包的最大大小
#define XNET_CFG_PACKGE_MAX_SIZE 1516
// 网络数据结构
typedef struct _xnet_packet_t {
	// 包中有效数据大小
	uint16_t size;
	// 包中数据起始地址
	uint8_t data;
	// 最大负载数据量
	uint8_t payload[XNET_CFG_PACKGE_MAX_SIZE];
}xnet_packet_t;

xnet_packet_t* xnet_alloc_for_send(uint16_t data_size);
xnet_packet_t* xnet_alloc_for_read(uint16_t data_size);

void xnet_init(void);
void xnet_poll(void);

#endif 