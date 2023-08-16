#ifndef XSERVER_HTTP_H
#define XSERVER_HTTP_H
#include "xnet_tiny.h"

#define XHTTP_DOC_DIR "htdocs"
xnet_err_t xserver_http_create(uint16_t port);
void xserver_http_run(void);
#endif // XSERVER_HTTP_H
