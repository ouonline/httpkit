#ifndef __HTTPKIT_KVPAIR_H__
#define __HTTPKIT_KVPAIR_H__

#include "httpkit/http_item.h"
#include "cutils/cvector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kvpair {
    struct http_item key;
    struct http_item value;
};

int kvpair_vector_update(struct cvector*, const void* base, unsigned int koff, unsigned int klen,
                         unsigned int voff, unsigned int vlen);

struct kvpair* kvpair_vector_lookup(struct cvector*, const void* base, const char* key,
                                    unsigned int klen);

#ifdef __cplusplus
}
#endif

#endif
