#include <stdio.h> /* snprintf() */
#include "httpkit/http_response_encode.h"
#include "http_header_encode.h"
#include "def.h"

#define MAX_CODE_LEN 5

int http_response_encode_status_line(struct qbuf* res, unsigned int code,
                                     const char* text, unsigned int text_len) {
    char code_str[MAX_CODE_LEN + 1];
    int code_len = snprintf(code_str, MAX_CODE_LEN + 1, " %u ", code);

    int ret = qbuf_reserve(res, HTTP_VERSION_STR_LEN + text_len + MAX_CODE_LEN +
                           2 /* "\r\n" */);
    if (ret != 0) {
        return HRC_NOMEM;
    }

    qbuf_append(res, HTTP_VERSION_STR, HTTP_VERSION_STR_LEN);
    qbuf_append(res, code_str, code_len);
    qbuf_append(res, text, text_len);
    qbuf_append(res, "\r\n", 2);

    return HRC_OK;
}

int http_response_encode_header(struct qbuf* res, const char* key, unsigned int klen,
                                const char* value, unsigned int vlen) {
    return http_header_encode(res, key, klen, value, vlen);
}
