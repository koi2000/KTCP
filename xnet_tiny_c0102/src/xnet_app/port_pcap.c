﻿#include <string.h>
#include <stdlib.h>
#include "pcap_device.h"
#include "xnet_tiny.h"

static pcap_t* pcap;

// pcap所用的网卡
const char* ip_str = "192.168.254.1";
const char my_mac_addr[] = { 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88 };


/*
* 初始化网络驱动
* @return 0成功，其他失败
*/
xnet_err_t xnet_driver_open(uint8_t * mac_addr){
	memcpy(mac_addr, my_mac_addr, sizeof(my_mac_addr));
	pcap = pcap_device_open(ip_str, mac_addr, 1);
	if (pcap) {
		exit(-1);
	}
	return XNET_ERR_OK;
}

/*
* 发送数据
* @param frame 数据起始地址
* @param size 数据长度
* @return 0 成功，其他失败
*/
xnet_err_t xnet_driver_send(xnet_packet_t * packet){
	return pcap_device_send(pcap, packet->data, packet->size);
}

/**
 * 读取数据
 * @param frame 数据存储位置
 * @param size 数据长度
 * @return 0 - 成功，其它失败
 */
xnet_err_t xnet_driver_read(xnet_packet_t** packet) {
    uint16_t size;
    xnet_packet_t* r_packet = xnet_alloc_for_read(XNET_CFG_PACKGE_MAX_SIZE);

    size = pcap_device_read(pcap, r_packet->data, XNET_CFG_PACKGE_MAX_SIZE);
    if (size) {
        r_packet->size = size;
        *packet = r_packet;
        return XNET_ERR_OK;
    }

    return XNET_ERR_IO;
}
