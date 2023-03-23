#include "httpkit/http_common.h"
#include "http_header_decode.h"

int http_header_decode(const char* data, unsigned long len, const char* base,
                       struct http_kv_ol_list* l, unsigned long* offset) {
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

HEADER_ITEM_COLON:
    ++cursor;
    if (cursor == end) {
        return HRC_MORE_DATA;
    }
    if (*cursor == '\r' || *cursor == '\n') {
        return HRC_HEADER;
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
        if (*cursor == '\r' || *cursor == '\n') {
            return HRC_HEADER;
        }
    } while (*cursor == ' ' || *cursor == '\t');
    value_off = cursor - base;

HEADER_ITEM_VALUE:
    while (1) {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor == ' ' || *cursor == '\t') {
            value_len = cursor - base - value_off;
            /* space(s) after value */
            do {
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
            } while (*cursor == ' ' || *cursor == '\t');
            if (*cursor == '\r') {
                break;
            }
            return HRC_HEADER;
        }
        if (*cursor == '\n') {
            return HRC_HEADER;
        }
        if (*cursor == '\r') {
            value_len = cursor - base - value_off;
            break;
        }
    }

HEADER_ITEM_r:
    ++cursor;
    if (cursor == end) {
        return HRC_MORE_DATA;
    }
    if (*cursor != '\n') {
        return HRC_HEADER;
    }

    /* HEADER_ITEM_END */
    rc = http_kv_ol_list_update(l, base, key_off, key_len,
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
