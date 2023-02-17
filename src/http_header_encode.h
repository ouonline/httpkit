#ifndef __HTTPKIT_HTTP_HEADER_ENCODE_H__
#define __HTTPKIT_HTTP_HEADER_ENCODE_H__

#include "httpkit/http_kv_list.h"

int http_header_encode(const struct http_kv_list* header_list, unsigned long content_len,
                       struct qbuf* res);

#endif
