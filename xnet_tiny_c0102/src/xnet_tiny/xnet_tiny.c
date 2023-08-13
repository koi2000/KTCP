#include <string.h>    
#include "xnet_tiny.h"
#define min(a,b) ((a)>(b)?(b):(a))


static uint8_t netif_mac[XNET_MAC_ADDR_SIZE];
// �����뷢�ͻ�����
static xnet_packet_t tx_packet, rx_packet;

#define swap_order16(v) ((((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF))

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
* ��̫����ʼ��
* @return ��ʼ�����
*/
static xnet_err_t ethernet_init(void){
	xnet_err_t err = xnet_driver_open(netif_mac);
	if (err < 0)return err;
	return XNET_ERR_OK;
}

/*
* ����һ����̫������֡
* @param protocol �ϲ�����Э�� IP��ARP
* @param mac_addr Ŀ��������mac��ַ
* @param packet �����͵����ݰ�
* @return ���ͽ��
*/
static xnet_err_t ethernet_out_to(xnet_protocol_t protocol,const uint8_t*mac_addr,xnet_packet_t *packet) {
	xether_hdr_t* ether_hdr;
	// ���ͷ��
	add_header(packet, sizeof(xether_hdr_t));
	ether_hdr = (xether_hdr_t*)packet->data;
	// ��ͷ������Ŀ��mac��ַ
	memcpy(ether_hdr->dest, mac_addr, XNET_MAC_ADDR_SIZE);
	memcpy(ether_hdr->src, netif_mac, XNET_MAC_ADDR_SIZE);
	ether_hdr->protocol = swap_order16(protocol);
	// ���ݷ���
	return xnet_driver_send(packet);
}

/*
* ��̫������֡�������
* @param packet ������İ�
*/
static void ethernet_in(xnet_packet_t* packet) {
	// ����Ҫ��ͷ�����ݴ�
	if (packet->size <= sizeof(xether_hdr_t)) {
		return;
	}
	// �鿴ͷ����ʽ�������ϲ�Э�鴦��
	xether_hdr_t* hdr = (xether_hdr_t*)packet->data;
	switch (swap_order16(hdr->protocol)) {
	case XNET_PROTOCOL_ARP:
		break;
	case XNET_PROTOCOL_IP:
		break;
	}
}

/*
* ��ѯ����ӿڣ������Ƿ������ݰ���������д���
*/
static void ethernet_poll(void) {
	xnet_packet_t* packet;
	if (xnet_driver_read(&packet) == XNET_ERR_OK) {
		// �Է�ping 192.168.254.2ʱ����ͣ�ڴ˴�
		ethernet_in(packet);
	}
}

/*
* Э��ջ�ĳ�ʼ��
*/
void xnet_init(void) {
	ethernet_init();
}

/*
* ��ѯ�������ݰ�������Э��ջ�д���
*/
void xnet_poll(void){
	ethernet_poll();
}