#ifndef XNET_TINT_H
#define XNET_TINY_H

#include <stdint.h>
// �շ����ݰ�������С
#define XNET_CFG_PACKGE_MAX_SIZE 1516

typedef enum _xnet_err_t {
	XNET_ERR_OK = 0,
	XNET_ERR_IO = -1,
}xnet_err_t;

// �������ݽṹ
typedef struct _xnet_packet_t {
	// ������Ч���ݴ�С
	uint16_t size;
	// ����������ʼ��ַ
	uint8_t data;
	// �����������
	uint8_t payload[XNET_CFG_PACKGE_MAX_SIZE];
}xnet_packet_t;

xnet_packet_t* xnet_alloc_for_send(uint16_t data_size);
xnet_packet_t* xnet_alloc_for_read(uint16_t data_size);

void xnet_init(void);
void xnet_poll(void);

#endif 