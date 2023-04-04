#ifndef __HTTPKIT_HTTP_REQUEST_ENCODE_H__
#define __HTTPKIT_HTTP_REQUEST_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/qbuf.h"
#include "http_common.h"

int http_request_encode_request_line(struct qbuf* res, const char* method, unsigned int method_len,
                                     const char* url, unsigned int url_len);

int http_request_encode_header(struct qbuf* res, const char* key, unsigned int klen,
                               const char* value, unsigned int vlen);

static inline int http_request_encode_head_end(struct qbuf* res) {
    int ret = qbuf_append(res, "\r\n", 2);
    return (ret == 0) ? HRC_OK : HRC_NOMEM;
}

#ifdef __cplusplus
}
#endif

#endif
