#ifndef __HTTPKIT_HTTP_UTILS_H__
#define __HTTPKIT_HTTP_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/qbuf.h"

int http_url_decode(const char* src, unsigned int src_size, struct qbuf* dst);

struct http_response_status {
    unsigned int code;
    const char* code_str;
    unsigned int code_len;
    const char* text_str;
    unsigned int text_len;
};

#ifdef __cplusplus
typedef struct http_response_status HttpResponseStatus;
#endif

const struct http_response_status* http_response_status_lookup(unsigned int code);

#ifdef __cplusplus
}
#endif

#endif
