#ifndef __HTTPKIT_HTTP_REQUEST_ENCODE_H__
#define __HTTPKIT_HTTP_REQUEST_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http_kv_list.h"

int http_request_encode_head(const struct qbuf_ref* method, const struct qbuf_ref* abs_path,
                             const struct http_kv_list* query_list,
                             const struct http_kv_list* header_list,
                             unsigned long content_len, struct qbuf* res);

#ifdef __cplusplus
}
#endif

#endif
