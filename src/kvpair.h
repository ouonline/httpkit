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

int kvpair_vector_update(struct cvector*, const void* base, unsigned long koff, unsigned long klen,
                         unsigned long voff, unsigned vlen);

int kvpair_vector_foreach(struct cvector*, const void* base, void* arg,
                          int (*f)(void* arg, const char* key, unsigned int klen,
                                   const char* value, unsigned int vlen));

struct kvpair* kvpair_vector_lookup(struct cvector*, const void* base,
                                    const char* key, unsigned int klen);

#ifdef __cplusplus
}
#endif

#endif