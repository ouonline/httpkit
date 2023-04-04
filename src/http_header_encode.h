#ifndef __HTTPKIT_HTTP_HEADER_ENCODE_H__
#define __HTTPKIT_HTTP_HEADER_ENCODE_H__

#include "cutils/qbuf.h"

int http_header_encode(struct qbuf* res, const char* key, unsigned int klen,
                       const char* value, unsigned int vlen);

#endif
