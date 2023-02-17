#ifndef __HTTPKIT_HTTP_REQUEST_ENCODE_H__
#define __HTTPKIT_HTTP_REQUEST_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http_kv_list.h"

int http_request_encode(const struct qbuf_ref* method, const struct qbuf_ref* abs_path,
                        const struct http_kv_list* option_list,
                        const struct http_kv_list* header_list,
                        const char* content, unsigned long content_len,
                        struct qbuf* res);

#ifdef __cplusplus
}
#endif

#endif
