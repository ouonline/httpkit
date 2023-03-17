#include "httpkit/http_common.h"
#include "http_header_decode.h"
#include "cutils/str_utils.h" /* ndec2long()/memmem() */
#include <string.h> /* memcmp() */

static int __header_decode(const char* data, unsigned long len, const char* base,
                           struct http_kv_ol_list* l) {
    const char* cursor = memmem(data, len, ":", 1);
    if (!cursor || cursor == data) {
        return HRC_HEADER;
    }

    /* skips space(s) before ':' */
    const char* key_end;
    for (key_end = cursor; key_end > data && *(key_end - 1) == ' '; --key_end);

    unsigned int klen = key_end - data;

    /* skips space(s) */
    const char* end = data + len;
    for (++cursor /* skip ':' */; cursor < end && *cursor == ' '; ++cursor);
    /* now cursor points to the beginning of header value */
    unsigned int vlen = len - (cursor - data);

    return http_kv_ol_list_update(l, base, data - base, klen, cursor - base, vlen);
}


int http_header_decode(const char* data, unsigned long len, const char* base,
                       struct http_kv_ol_list* l, unsigned long* offset) {
    while (1) {
        const char* cursor = memmem(data, len, "\r\n", 2);
        if (!cursor) {
            return HRC_MORE_DATA;
        }

        /* end of headers */
        if (cursor == data) {
            (*offset) += 2;
            return HRC_OK;
        }

        int rc = __header_decode(data, cursor - data, base, l);
        if (rc != HRC_OK) {
            return rc;
        }

        len -= (cursor + 2 - data);
        (*offset) += (cursor + 2 - data);
        data = cursor + 2;
    }
}
