#ifndef __HTTPKIT_HTTP_HEADER_DECODE_H__
#define __HTTPKIT_HTTP_HEADER_DECODE_H__

#include "httpkit/http_kv_ol_list.h"

int http_header_decode(const char* data, unsigned long len, const char* base,
                       struct http_kv_ol_list*, unsigned long* offset);

#endif
