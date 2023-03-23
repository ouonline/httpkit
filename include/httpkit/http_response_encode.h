#ifndef __HTTPKIT_HTTP_RESPONSE_ENCODE_H__
#define __HTTPKIT_HTTP_RESPONSE_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http_kv_list.h"

int http_response_encode_head(unsigned int status_code,
                              const char* text, unsigned int text_len,
                              const struct http_kv_list* header_list,
                              unsigned long content_len, struct qbuf* res);

#ifdef __cplusplus
}
#endif

#endif
