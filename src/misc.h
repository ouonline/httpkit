#ifndef __HTTPKIT_MISC_H__
#define __HTTPKIT_MISC_H__

#include "cutils/str_utils.h"
#include "def.h"
#include "kvpair.h"
#include <string.h>

static inline int __check_content_length(void* arg, const char* k, unsigned int klen,
                                         const char* v, unsigned int vlen) {
    unsigned long* content_len = arg;
    if (klen == CONTENT_LENGTH_LEN && memcmp(CONTENT_LENGTH_STR, k, CONTENT_LENGTH_LEN) == 0) {
        *content_len = ndec2long(v, vlen);
        return 1;
    }
    return 0;
}

static inline void set_content_len(const char* base, struct cvector* hdr_list, unsigned long* content_len) {
    int ret = kvpair_vector_foreach(hdr_list, base, content_len, __check_content_length);
    if (ret == 0) {
        *content_len = 0;
    }
}

#endif
