#ifndef __HTTPKIT_MISC_H__
#define __HTTPKIT_MISC_H__

#include "cutils/str_utils.h"
#include "def.h"
#include "httpkit/http_retcode.h"
#include "kvpair.h"
#include <limits.h>
#include <string.h>

static inline int set_content_len(const char* base, struct cvector* hdr_list,
                                  unsigned long* content_len) {
    struct kvpair* item = kvpair_vector_lookup_case_insensitive(
        hdr_list, base, CONTENT_LENGTH_STR, CONTENT_LENGTH_LEN);
    if (item) {
        const char* value = base + item->value.off;
        unsigned long len = item->value.len;
        unsigned long result = 0;

        if (len == 0) {
            return HRC_CONTENTLEN;
        }

        for (unsigned long j = 0; j < len; ++j) {
            unsigned char c = (unsigned char)value[j];
            if (c < '0' || c > '9') {
                return HRC_CONTENTLEN;
            }
            if (result > (ULONG_MAX - (c - '0')) / 10) {
                return HRC_CONTENTLEN;
            }
            result = result * 10 + (c - '0');
        }

        *content_len = result;
        return HRC_OK;
    }

    *content_len = 0;
    return HRC_OK;
}

#endif
