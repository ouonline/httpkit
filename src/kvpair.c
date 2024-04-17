#include "httpkit/http_retcode.h"
#include "kvpair.h"
#include <string.h>

int kvpair_vector_update(struct cvector* vec, const void* base, unsigned int koff, unsigned int klen,
                         unsigned int voff, unsigned int vlen) {
    int ret;
    unsigned int vec_size = cvector_size(vec);
    for (unsigned int i = 0; i < vec_size; ++i) {
        struct kvpair* item = (struct kvpair*)cvector_at(vec, i);
        if (item->key.len != klen) {
            continue;
        }
        if (memcmp((const char*)base + koff, (const char*)base + item->key.off, klen) != 0) {
            continue;
        }

        item->value.off = voff;
        item->value.len = vlen;
        return HRC_OK;
    }

    struct kvpair new_item = {
        .key = {
            .off = koff,
            .len = klen,
        },
        .value = {
            .off = voff,
            .len = vlen,
        },
    };

    ret = cvector_push_back(vec, struct kvpair, new_item);
    if (ret != 0) {
        return HRC_NOMEM;
    }

    return HRC_OK;
}

struct kvpair* kvpair_vector_lookup(struct cvector* vec, const void* base, const char* key,
                                    unsigned int klen) {
    unsigned int vec_size = cvector_size(vec);
    for (unsigned int i = 0; i < vec_size; ++i) {
        struct kvpair* item = (struct kvpair*)cvector_at(vec, i);
        if (item->key.len == klen && memcmp(key, (const char*)base + item->key.off, klen) == 0) {
            return item;
        }
    }
    return NULL;
}
