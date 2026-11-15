#include "httpkit/http_retcode.h"
#include "def.h"
#include "kvpair.h"
#include "http_header_decode.h"
#include <string.h>

static int __check_content_len_conflict(
    const char* base, struct cvector* kvpair_list, unsigned long key_off,
    unsigned long key_len, unsigned long value_off, unsigned long value_len) {
    if (key_len != CONTENT_LENGTH_LEN ||
        memcmp(base + key_off, CONTENT_LENGTH_STR, CONTENT_LENGTH_LEN) != 0) {
        return HRC_OK;
    }

    struct kvpair* item = kvpair_vector_lookup(
        kvpair_list, base, CONTENT_LENGTH_STR, CONTENT_LENGTH_LEN);
    if (item == NULL) {
        return HRC_OK;
    }

    if (item->value.len != value_len ||
        memcmp(base + item->value.off, base + value_off, value_len) != 0) {
        return HRC_CONTENTLEN;
    }

    return HRC_OK;
}

int http_header_decode(const char* data, unsigned long len, const char* base,
                       struct cvector* kvpair_list, unsigned long* offset) {
    int rc;
    const char* cursor = data;
    const char* end = data + len;
    unsigned long key_off, key_len, value_off, value_len;

HEADER_ITEM_BEGIN:
    key_off = key_len = value_off = value_len = 0;
    while (1) {
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor != ' ' && *cursor != '\t') {
            break;
        }
        ++cursor;
    }
    if (*cursor == ':' /* empty key */ || *cursor == '\n') {
        return HRC_HEADER;
    }
    if (*cursor == '\r') {
        goto HEADER_ITEM_EMPTY_r; /* empty header */
    }
    key_off = cursor - base;

    /* HEADER_ITEM_KEY */
    while (1) {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor == '\r' || *cursor == '\n') {
            return HRC_HEADER;
        }
        if (*cursor == ' ' || *cursor == '\t') {
            key_len = cursor - base - key_off;
            /* space(s) between key and colon */
            do {
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
            } while (*cursor == ' ' || *cursor == '\t');
            if (*cursor == ':') {
                break;
            }
            return HRC_HEADER;
        }
        if (*cursor == ':') {
            key_len = cursor - base - key_off;
            break;
        }
    }

    /* HEADER_ITEM_COLON */
    ++cursor;
    if (cursor == end) {
        return HRC_MORE_DATA;
    }
    if (*cursor == '\n') {
        return HRC_HEADER;
    }
    if (*cursor == '\r') {
        value_off = cursor - base;
        goto HEADER_ITEM_r; /* empty value */
    }
    if (*cursor != ' ' && *cursor != '\t') {
        value_off = cursor - base;
        goto HEADER_ITEM_VALUE;
    }

    /* space(s) between colon and value */
    do {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor == '\n') {
            return HRC_HEADER;
        }
    } while (*cursor == ' ' || *cursor == '\t');
    value_off = cursor - base;
    if (*cursor == '\r') {
        goto HEADER_ITEM_r; /* empty value */
    }

HEADER_ITEM_VALUE:
    const char* last_non_space = cursor;
    while (1) {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor == '\n') {
            return HRC_HEADER;
        }
        if (*cursor == '\r') {
            value_len = last_non_space + 1 - base - value_off;
            break;
        }
        if (*cursor != ' ' && *cursor != '\t') {
            last_non_space = cursor;
        }
    }

HEADER_ITEM_r:
    /* HEADER_ITEM_r */
    ++cursor;
    if (cursor == end) {
        return HRC_MORE_DATA;
    }
    if (*cursor != '\n') {
        return HRC_HEADER;
    }

    /* HEADER_ITEM_END */
    rc = __check_content_len_conflict(base, kvpair_list, key_off, key_len,
                                      value_off, value_len);
    if (rc != HRC_OK) {
        return rc;
    }

    rc = kvpair_vector_update(kvpair_list, (void*)base, key_off, key_len,
                              value_off, value_len);
    if (rc != HRC_OK) {
        return rc;
    }

    ++cursor; /* skips '\n' */
    (*offset) += (cursor - data);
    data = cursor;
    goto HEADER_ITEM_BEGIN;

HEADER_ITEM_EMPTY_r:
    ++cursor;
    if (cursor == end) {
        return HRC_MORE_DATA;
    }
    if (*cursor == '\n') {
        (*offset) += (cursor - data) + 1;
        return HRC_OK;
    }
    return HRC_HEADER;
}
