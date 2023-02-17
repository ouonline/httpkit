#ifndef __HTTPKIT_HTTP_RESPONSE_ENCODE_H__
#define __HTTPKIT_HTTP_RESPONSE_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http_kv_list.h"

int http_response_encode(unsigned int status_code, struct http_kv_list* header_list,
                         const char* content, unsigned long content_len, struct qbuf* res);

#ifdef __cplusplus
}
#endif

#endif
