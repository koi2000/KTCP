#include <string.h>    
#include "xnet_tiny.h"
#define min(a,b) ((a)>(b)?(b):(a))

static const xipaddr_t netif_ipaddr = XNET_CFG_NETIF_IP;
static const uint8_t ether_broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t netif_mac[XNET_MAC_ADDR_SIZE];
// �����뷢�ͻ�����
static xnet_packet_t tx_packet, rx_packet;
// ��ʡ�ڴ棬ֻʹ��һ������
static xarp_entry_t arp_entry;
static xnet_time_t arp_timer;
static void update_arp_entry(uint8_t* src_ip, uint8_t* mac_addr);

#define swap_order16(v) ((((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF))
#define xipaddr_is_equal_buf(addr,buf) (memcmp((addr)->array,(buf),XNET_IPV4_ADDR_SIZE)==0)
#define xipaddr_is_equal(addr1,addr2) ((addr1)->addr==(addr2)->addr)
#define xipaddr_from_buf(dest,buf) ((dest)->addr=*(uint32_t*)(buf))
/*
* ����Ƿ�ʱ
* @param time ǰһʱ��
* @param sec Ԥ�ڳ�ʱʱ�䣬ֵΪ0ʱ����ʾ��ȡ��ǰʱ��
* @return 0-δ��ʱ��1-��ʱ
*/
int xnet_check_tmo(xnet_time_t*time,uint32_t sec){
	xnet_time_t curr = xsys_get_time();
	// 0ȡ��ǰʱ��
	if (sec == 0) {
		*time = curr;
		return 0;
	}else if (curr - *time >= sec) {
		// ��0��鳬ʱ
		*time = curr;
		return 1;
	}
	return 0;
}

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
	// ����ץ������wireshark�����ڴ��ڷ����������ݰ�ץȡ
	// 1	0.000000	Dell_f9:e6:77	Broadcast	ARP	42	ARP Announcement for 192.168.137.2
	return xarp_make_request(&netif_ipaddr);
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
* ��IP���ݰ�ͨ����̫�����ͳ�ȥ
* @param dest_ip Ŀ��IP��ַ
* @param packet ������IP���ݰ�
* @return ���ͽ��
*/
static xnet_err_t ethernet_out(xipaddr_t* dest_ip, xnet_packet_t* packet) {
	xnet_err_t err;
	uint8_t* mac_addr;
	if ((err = xarp_resolve(dest_ip, &mac_addr)) == XNET_ERR_OK) {
		return ethernet_out_to(XNET_PROTOCOL_IP, mac_addr, packet);
	}
	return err;
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
		remove_header(packet, sizeof(xether_hdr_t));
		xarp_in(packet);
		break;
	case XNET_PROTOCOL_IP:
		// ��IP��ͷ����ȡIP��ַ������̫����ͷ����ȡmac��ַ
		// ����ARP��
#if 0
		xip_hdr_t* iphdr = (xip_hdr_t*)(packet->data + sizeof(xether_hdr_t));
		if (packet->size >= sizeof(xether_hdr_t) + sizeof(xip_hdr_t)) {
			if (memcmp(iphdr->dest_ip, &netif_ipaddr.array, XNET_IPV4_ADDR_SIZE) == 0) {
				update_arp_entry(iphdr->src_ip, hdr->src);
			}
		}
#endif
		remove_header(packet, sizeof(xether_hdr_t));
		xip_in(packet);
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
* ARP��ʼ��
*/
void xarp_init(void) {
	arp_entry.state = XARP_ENTRY_FREE;
	// ��ȡ��ʼʱ��
	xnet_check_tmo(&arp_timer, 0);
}

/*
* ��ѯARP�����Ƿ�ʱ����ʱ����������
*/
void xarp_poll(void) {
	if (xnet_check_tmo(&arp_timer, XARP_TIMER_PERIOD)) {
		switch (arp_entry.state) {
		case XARP_ENTRY_RESOLVING:
			if (--arp_entry.tmo == 0) {
				if (arp_entry.retry_cnt-- == 0) {
					arp_entry.state = XARP_ENTRY_FREE;
				}else{
					xarp_make_request(&arp_entry.ipaddr);
					arp_entry.state = XARP_ENTRY_RESOLVING;
					arp_entry.tmo = XARP_CFG_ENTRY_PENDING_TMO;
				}
			}
			break;
		case XARP_ENTRY_OK:
			if (--arp_entry.tmo == 0) {     // ��ʱ����������
				xarp_make_request(&arp_entry.ipaddr);
				arp_entry.state = XARP_ENTRY_RESOLVING;
				arp_entry.tmo = XARP_CFG_ENTRY_PENDING_TMO;
			}
			break;
		}
	}
}

/*
* ����һ��ARP��Ӧ
* @param arp_packet ���յ�ARP�����
* @return ���ɽ��
*/
xnet_err_t xarp_make_response(xarp_packet_t* arp_packet) {
	xarp_packet_t* response_packet;
	xnet_packet_t* packet = xnet_alloc_for_send(sizeof(xarp_packet_t));
	response_packet = (xarp_packet_t*)packet->data;;
	response_packet->hw_type = swap_order16(XARP_HW_ETHER);
	response_packet->pro_type = swap_order16(XNET_PROTOCOL_IP);
	response_packet->hw_len = XNET_MAC_ADDR_SIZE;
	response_packet->pro_len = XNET_IPV4_ADDR_SIZE;
	response_packet->opcode = swap_order16(XARP_REPLY);
	memcpy(response_packet->target_mac, arp_packet->sender_mac, XNET_MAC_ADDR_SIZE);
	memcpy(response_packet->target_ip, arp_packet->sender_ip, XNET_IPV4_ADDR_SIZE);
	memcpy(response_packet->sender_mac, netif_mac, XNET_MAC_ADDR_SIZE);
	memcpy(response_packet->sender_ip, netif_ipaddr.array, XNET_IPV4_ADDR_SIZE);
	return ethernet_out_to(XNET_PROTOCOL_ARP, ether_broadcast, packet);
}

/*
* ����һ��ARP������������ָ��ip��ַ�Ļ�������һ��ARP��Ӧ
* @param ipaddr �����IP��ַ
* @return ������
*/
xnet_err_t xarp_make_request(const xipaddr_t * ipaddr){
	xarp_packet_t* arp_packet;
	xnet_packet_t* packet = xnet_alloc_for_send(sizeof(xarp_packet_t));
	arp_packet = (xarp_packet_t*)packet->data;
	arp_packet->hw_type = swap_order16(XARP_HW_ETHER);
	arp_packet->pro_type = swap_order16(XNET_PROTOCOL_IP);
	arp_packet->hw_len = XNET_MAC_ADDR_SIZE;
	arp_packet->pro_len = XNET_IPV4_ADDR_SIZE;
	arp_packet->opcode = swap_order16(XARP_REQUEST);
	memcpy(arp_packet->sender_mac, netif_mac, XNET_MAC_ADDR_SIZE);
	memcpy(arp_packet->sender_ip, netif_ipaddr.array, XNET_IPV4_ADDR_SIZE);
	memset(arp_packet->target_mac, 0, XNET_MAC_ADDR_SIZE);
	memcpy(arp_packet->target_ip, ipaddr->array, XNET_IPV4_ADDR_SIZE);
	return ethernet_out_to(XNET_PROTOCOL_ARP, ether_broadcast, packet);
}

/*
* ����ָ����IP��ַ���������ARP�����У�����ARP����
* @param ipaddr ���ҵ�ip��ַ
* @param mac_addr ���ص�mac��ַ�洢��
* @return XNET_ERR_OK ���ҳɹ���XNET_ERR_NONE ����ʧ��
*/
xnet_err_t xarp_resolve(const xipaddr_t* ipaddr,uint8_t** mac_addr){
	if ((arp_entry.state == XARP_ENTRY_OK) && xipaddr_is_equal(ipaddr, &arp_entry.ipaddr)) {
		*mac_addr = arp_entry.macaddr;
		return XNET_ERR_OK;
	}
	xarp_make_request(ipaddr);
	return XNET_ERR_NONE;
}

/*
* ����ARP����
* @param src_ip Դ��ַIP
* @param mac_addr ��Ӧ��mac��ַ
*/
static void update_arp_entry(uint8_t* src_ip, uint8_t* mac_addr) {
	memcpy(arp_entry.ipaddr.array, src_ip, XNET_IPV4_ADDR_SIZE);
	memcpy(arp_entry.macaddr, mac_addr, 6);
	arp_entry.state = XARP_ENTRY_OK;
	arp_entry.tmo = XARP_CFG_ENTRY_OK_TMO;
	arp_entry.retry_cnt = XARP_CFG_MAX_RETRIES;
}

/*
* ARP���봦��
* @param packet �����ARP��
*/
void xarp_in(xnet_packet_t* packet) {
	if (packet->size >= sizeof(xarp_packet_t)) {
		xarp_packet_t* arp_packet = (xarp_packet_t*)packet->data;
		uint16_t opcode = swap_order16(arp_packet->opcode);

		// ���ĺϷ��Լ��
		if ((swap_order16(arp_packet->hw_type) != XARP_HW_ETHER) ||
			(arp_packet->hw_len != XNET_MAC_ADDR_SIZE) ||
			(swap_order16(arp_packet->pro_type) != XNET_PROTOCOL_IP) ||
			(arp_packet->pro_len != XNET_IPV4_ADDR_SIZE)
			|| ((opcode != XARP_REQUEST) && (opcode != XARP_REPLY))) {
			return;
		}

		// ֻ�������Լ����������Ӧ��
		if (!xipaddr_is_equal_buf(&netif_ipaddr, arp_packet->target_ip)) {
			return;
		}

		// ���ݲ�������в�ͬ�Ĵ���
		switch (swap_order16(arp_packet->opcode)) {
		case XARP_REQUEST:  // ���󣬻�����Ӧ
			// �ڶԷ�����Ping �Լ���Ȼ��wireshark���ܿ���ARP�������Ӧ
			// ���������ܿ��ܶԷ�Ҫ���Լ�ͨ�ţ����Ը���һ��
			xarp_make_response(arp_packet);
			update_arp_entry(arp_packet->sender_ip, arp_packet->sender_mac);
			break;
		case XARP_REPLY:    // ��Ӧ�������Լ��ı�
			update_arp_entry(arp_packet->sender_ip, arp_packet->sender_mac);
			break;
		}
	}
}

/*
* У��ͼ���
* @param buf У������������ʼ��ַ
* @param len �������ĳ��ȣ����ֽ�Ϊ��λ
* @param pre_sum �ۼӵ�֮ǰ��ֵ�����ڶ�ε���checksum
* @param complement �Ƿ���ۼӺ͵Ľ������ȡ��
* @return У��ͽ��
*/
static uint16_t checksum16(uint16_t*buf,uint16_t len,uint16_t pre_sum,int complement){
	uint32_t checksum = pre_sum;
	uint16_t high;
	while (len > 1) {
		checksum += *buf++;
		len -= 2;
	}
	if (len > 0) {
		checksum += *(uint8_t*)buf;
	}
	while ((high = checksum >> 16) != 0) {
		checksum = high + (checksum & 0xffff);
	}
	return complement ? (uint16_t)~checksum : (uint16_t)checksum;
}

/*
* IP�ĳ�ʼ��
*/
void xip_init(void){}

/*
* IP������봦��
* @param packet �����IP���ݰ�
*/
void xip_in(xnet_packet_t* packet) {
	xip_hdr_t* iphdr = (xip_hdr_t*)packet->data;
	uint32_t total_size, header_size;
	uint16_t pre_checksum;
	xipaddr_t src_ip;
	// ���汾��
	if (iphdr->version != XNET_VERSION_IPV4) {
		return;
	}
	// ��鳤��
	header_size = iphdr->hdr_len * 4;
	total_size = swap_order16(iphdr->total_len);
	if ((header_size < sizeof(xip_hdr_t)) || ((total_size < header_size) || (packet->size < total_size))) {
		return;
	}

	// У���Ҫ����
	pre_checksum = iphdr->hdr_checksum;
	iphdr->hdr_checksum = 0;
	if (pre_checksum != checksum16((uint16_t*)iphdr, header_size, 0, 1)) {
		return;
	}
	// ֻ����Ŀ��IPΪ�Լ������ݰ��������㲥֮���IPȫ������
	if (!xipaddr_is_equal_buf(&netif_ipaddr, iphdr->dest_ip)) {
		return;
	}
	xipaddr_from_buf(&src_ip, iphdr->src_ip);
	// �ֱ���ICMP��UDP��TCP����
	switch (iphdr->protocol) {
	case XNET_PROTOCOL_ICMP:
		remove_header(packet, header_size);
		xicmp_in(&src_ip, packet);
		break;
	default:
		// Э�鲻�ɴû�������κ�Э���ܴ�����������ݰ�
		xicmp_dest_unreach(XICMP_CODE_PRO_UNREACH, iphdr);
		break;
	}
}

/*
* IP�������
* @param protocol �ϲ�Э�� ICMP UDP��TCP
* @param dest_ip
* @param packet
* @return
*/
xnet_err_t xip_out(xnet_protocol_t protocol, xipaddr_t* dest_ip, xnet_packet_t* packet) {
	static uint32_t ip_packet_id = 0;
	xip_hdr_t* iphdr;
	add_header(packet, sizeof(xip_hdr_t));
	iphdr = (xip_hdr_t*)packet->data;
	iphdr->version = XNET_VERSION_IPV4;
	iphdr->hdr_len = sizeof(xip_hdr_t) / 4;
	iphdr->tos = 0;
	iphdr->total_len = swap_order16(packet->size);
	iphdr->id = swap_order16(ip_packet_id);
	iphdr->flags_fragment = 0;
	iphdr->ttl = XNET_IP_DEFAULT_TTL;
	iphdr->protocol = protocol;
	memcpy(iphdr->dest_ip, dest_ip->array, XNET_IPV4_ADDR_SIZE);
	memcpy(iphdr->src_ip, netif_ipaddr.array, XNET_IPV4_ADDR_SIZE);
	iphdr->hdr_checksum = 0;
	iphdr->hdr_checksum = checksum16((uint16_t*)iphdr, sizeof(xip_hdr_t), 0, 1);;

	ip_packet_id++;
	return ethernet_out(dest_ip, packet);
}

/*
* ICMP��ʼ��
*/
void xicmp_init(void) {}

/*
* ����ICMP ECHO��Ӧ ����Ӧping
* @param icmp_hdr �յ���icmp��ͷ
* @param src_ip ������Դip
* @param packet �յ������ݰ�
* @return ������
*/
static xnet_err_t reply_icmp_request(xicmp_hdr_t* icmp_hdr, xipaddr_t* src_ip, xnet_packet_t* packet) {
	xicmp_hdr_t* reply_hdr;
	xnet_packet_t* tx = xnet_alloc_for_send(packet->size);
	reply_hdr = (xicmp_hdr_t*)tx->data;
	reply_hdr->type = XICMP_CODE_ECHO_REPLY;
	reply_hdr->code = 0;
	reply_hdr->id = icmp_hdr->id;
	reply_hdr->seq = icmp_hdr->seq;
	reply_hdr->checksum = 0;
	memcpy(((uint8_t*)reply_hdr) + sizeof(xicmp_hdr_t), ((uint8_t*)icmp_hdr) + sizeof(xicmp_hdr_t),
		packet->size - sizeof(xicmp_hdr_t));
	reply_hdr->checksum = checksum16((uint16_t*)reply_hdr, tx->size, 0, 1);
	return xip_out(XNET_PROTOCOL_ICMP, src_ip, tx);
}

/*
* ICMP�����봦��
* @param src_ip ���ݰ���Դ
* @param packet ����������ݰ�
*/
void xicmp_in(xipaddr_t* src_ip, xnet_packet_t* packet) {
	xicmp_hdr_t* icmphdr = (xicmp_hdr_t*)packet->data;
	if ((packet->size >= sizeof(xicmp_hdr_t)) && (icmphdr->type == XICMP_CODE_ECHO_REQUEST)) {
		reply_icmp_request(icmphdr, src_ip, packet);
	}
}

/*
* ����ICMP�˿ڲ��ɴ��Э�鲻�ɴ����Ӧ
* @param code ���ɴ��������
* @param ip_hdr �յ���ip��
* @return ������
*/
xnet_err_t xicmp_dest_unreach(uint8_t code, xip_hdr_t* ip_hdr) {
	xicmp_hdr_t* icmp_hdr;
	xipaddr_t dest_ip;
	xnet_packet_t* packet;
	// ����Ҫ������ip������
	uint16_t ip_hdr_size = ip_hdr->hdr_len * 4;
	uint16_t ip_data_size = swap_order16(ip_hdr->total_len) - ip_hdr_size;
	// RFC�ĵ���д����8�ֽڣ���ʵ�ʲ���windows��ֹ8���ֽ�
	ip_data_size = ip_data_size + min(ip_data_size, 8);
	// �������ݰ���Ȼ����
	packet = xnet_alloc_for_send(ip_data_size + sizeof(xicmp_hdr_t));
	icmp_hdr = (xicmp_hdr_t*)packet->data;
	icmp_hdr->type = XICMP_TYPE_UNREACH;
	icmp_hdr->code = code;
	icmp_hdr->checksum = 0;
	icmp_hdr->id = icmp_hdr->seq = 0;
	memcpy(((uint8_t*)icmp_hdr) + sizeof(xicmp_hdr_t), ip_hdr, ip_data_size);
	icmp_hdr->checksum = 0;
	icmp_hdr->checksum = checksum16((uint16_t*)icmp_hdr, packet->size, 0, 1);
	xipaddr_from_buf(&dest_ip, ip_hdr->src_ip);
	return xip_out(XNET_PROTOCOL_ICMP, &dest_ip, packet);
}


/*
* Э��ջ�ĳ�ʼ��
*/
void xnet_init(void) {
	ethernet_init();
	xarp_init();
	xip_init();
	xicmp_init();
}

/*
* ��ѯ�������ݰ�������Э��ջ�д���
*/
void xnet_poll(void){
	ethernet_poll();
	xarp_poll();
}