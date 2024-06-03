#ifndef __HTTPKIT_KVPAIR_H__
#define __HTTPKIT_KVPAIR_H__

#include "cutils/offlen.h"
#include "cutils/cvector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kvpair {
    struct offlen key;
    struct offlen value;
};

int kvpair_vector_update(struct cvector*, const void* base, unsigned long koff, unsigned long klen,
                         unsigned long voff, unsigned long vlen);

struct kvpair* kvpair_vector_lookup(struct cvector*, const void* base, const char* key,
                                    unsigned long klen);

#ifdef __cplusplus
}
#endif

#endif
