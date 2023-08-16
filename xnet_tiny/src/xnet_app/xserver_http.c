#include "xserver_http.h"
#include <string.h>
#include <stdio.h>

static uint8_t tx_buffer[1024];

static xnet_err_t http_handler(xtcp_t* tcp, xtcp_conn_state_t state) {
	if (state == XTCP_CONN_CONNECTED) {
		printf("http connected.\n");
	}
	else if (state == XTCP_CONN_CLOSED) {
		printf("http closed.\n");
	}
	else if (state == XTCP_CONN_DATA_RECV) {
		uint8_t* data = tx_buffer;
		uint16_t read_size = xtcp_read(tcp, tx_buffer, sizeof(tx_buffer));
		while (read_size) {
			uint16_t curr_size = xtcp_write(tcp, data, read_size);
			data += curr_size;
			read_size -= curr_size;
		}
	}
	return XNET_ERR_OK;
}