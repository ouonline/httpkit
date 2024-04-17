#ifndef __HTTPKIT_MISC_H__
#define __HTTPKIT_MISC_H__

#include "cutils/str_utils.h"
#include "def.h"
#include "kvpair.h"
#include <string.h>

static inline void set_content_len(const char* base, struct cvector* hdr_list, unsigned long* content_len) {
    unsigned int sz = cvector_size(hdr_list);
    for (unsigned int i = 0; i < sz; ++i) {
        struct kvpair* item = (struct kvpair*)cvector_at(hdr_list, i);
        if (item->key.len == CONTENT_LENGTH_LEN &&
            memcmp(CONTENT_LENGTH_STR, base + item->key.off, CONTENT_LENGTH_LEN) == 0) {
            *content_len = ndec2long(base + item->value.off, item->value.len);
            return;
        }
    }

    *content_len = 0;
}

#endif
