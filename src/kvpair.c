#include "httpkit/http_retcode.h"
#include "kvpair.h"
#include <string.h>

int kvpair_vector_update(struct cvector* vec, const void* base,
                         unsigned long koff, unsigned long klen,
                         unsigned long voff, unsigned long vlen) {
    int ret;
    unsigned long vec_size = cvector_size(vec);
    for (unsigned long i = 0; i < vec_size; ++i) {
        struct kvpair* item = (struct kvpair*)cvector_at(vec, i);
        if (item->key.len != klen) {
            continue;
        }
        if (memcmp((const char*)base + koff, (const char*)base + item->key.off,
                   klen) != 0) {
            continue;
        }

        item->value.off = voff;
        item->value.len = vlen;
        return HRC_OK;
    }

    struct kvpair new_item = {
        .key =
            {
                .off = koff,
                .len = klen,
            },
        .value =
            {
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

struct kvpair* kvpair_vector_lookup(struct cvector* vec, const void* base,
                                    const char* key, unsigned long klen) {
    unsigned long vec_size = cvector_size(vec);
    for (unsigned long i = 0; i < vec_size; ++i) {
        struct kvpair* item = (struct kvpair*)cvector_at(vec, i);
        if (item->key.len == klen &&
            memcmp(key, (const char*)base + item->key.off, klen) == 0) {
            return item;
        }
    }
    return NULL;
}

static int __key_equal_case_insensitive(const char* a, const char* b,
                                        unsigned long len) {
    for (unsigned long i = 0; i < len; ++i) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca >= 'A' && ca <= 'Z') {
            ca = (unsigned char)(ca - 'A' + 'a');
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = (unsigned char)(cb - 'A' + 'a');
        }
        if (ca != cb) {
            return 0;
        }
    }
    return 1;
}

struct kvpair* kvpair_vector_lookup_case_insensitive(struct cvector* vec,
                                                     const void* base,
                                                     const char* key,
                                                     unsigned long klen) {
    unsigned long vec_size = cvector_size(vec);
    for (unsigned long i = 0; i < vec_size; ++i) {
        struct kvpair* item = (struct kvpair*)cvector_at(vec, i);
        if (item->key.len == klen &&
            __key_equal_case_insensitive(key, (const char*)base + item->key.off,
                                         klen)) {
            return item;
        }
    }
    return NULL;
}
