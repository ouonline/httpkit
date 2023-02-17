#ifndef __HTTPKIT_HTTP_UTILS_H__
#define __HTTPKIT_HTTP_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/qbuf.h"

int http_url_decode(const char* src, unsigned int src_size, struct qbuf* dst);

#ifdef __cplusplus
}
#endif

#endif
