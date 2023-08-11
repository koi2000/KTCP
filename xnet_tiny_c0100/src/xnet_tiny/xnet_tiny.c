#include "xnet_tiny.h"
#define min(a,b) ((a)>(b)?(b):(a))

static xnet_packet_t tx_packet, rx_packet;

/*
* ����һ���������ݰ����ڷ�������
* @param data_size ���ݿռ��С
* @return ����õ��İ�����
*/
xnet_packet_t * xnet_alloc_for_send(uint16_t data_size){
	// ��tx_packet�ĺ����ǰ���䣬��Ϊǰ��ҪԤ����Ϊ����Э���ͷ�����ݴ洢�ռ�
	tx_packet.data = tx_packet.payload + XNET_CFG_PACKGE_MAX_SIZE - data_size;
	tx_packet.size = data_size;
	return &tx_packet;
}

/*
* ����һ���������ݰ����ڶ�ȡ
* @param data_size ���ݿռ�Ĵ�С
* @return ����õ������ݰ�
*/
xnet_packet_t * xnet_alloc_for_read(uint16_t data_size){
	// ���ʼ���з��䣬������ײ����������֡��ȡ
	rx_packet.data = rx_packet.payload;
	rx_packet.size = data_size;
	return &rx_packet;
}

/*
* Ϊ�������һ��ͷ��
* @param packet ����������ݰ�
* @param header_size ���ӵ�ͷ����С
*/
static void add_header(xnet_packet_t *packet,uint16_t header_size){
	packet->data -= header_size;
	packet->size += header_size;
}

/*
* Ϊ�������ϴ�����ȥͷ��
* @param packet ����������ݰ�
* @param header_size ��ȥ��ͷ����С
*/
static void remove_header(xnet_packet_t *packet,uint16_t header_size) {
	packet->data += header_size;
	packet->size -= header_size;
}

/*
* �����ĳ��Ƚض�Ϊsize��С
* @param packet ����������ݰ�
* @param size ���մ�С
*/
static void truncate_packet(xnet_packet_t *packet,uint16_t size){
	packet->size = min(packet->size,size);
}

/*
* Э��ջ�ĳ�ʼ��
*/
void xnet_init(void){}

/*
* ��ѯ�������ݰ�������Э��ջ�д���
*/
void xnet_poll(void){}