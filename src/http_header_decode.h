#ifndef __HTTPKIT_HTTP_HEADER_DECODE_H__
#define __HTTPKIT_HTTP_HEADER_DECODE_H__

#include "cutils/cvector.h"

int http_header_decode(const char* data, unsigned long len, const char* base,
                       struct cvector* kvpair_list, unsigned long* offset);

#endif
