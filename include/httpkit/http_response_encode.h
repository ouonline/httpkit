#ifndef __HTTPKIT_HTTP_RESPONSE_ENCODE_H__
#define __HTTPKIT_HTTP_RESPONSE_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/qbuf.h"
#include "http_common.h"

int http_response_encode_status_line(struct qbuf* res, unsigned int code,
                                     const char* text, unsigned int text_len);

int http_response_encode_header(struct qbuf* res, const char* key, unsigned int klen,
                                const char* value, unsigned int vlen);

static inline int http_response_encode_head_end(struct qbuf* res) {
    int ret = qbuf_append(res, "\r\n", 2);
    return (ret == 0) ? HRC_OK : HRC_NOMEM;
}

#ifdef __cplusplus
}
#endif

#endif
