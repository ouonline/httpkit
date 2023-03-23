#include <stdio.h> /* snprintf() */
#include "httpkit/http_common.h"
#include "httpkit/http_response_encode.h"
#include "http_header_encode.h"

#define HTTP_VERSION_STR "HTTP/1.1"
#define HTTP_VERSION_STR_LEN 8

static void pack_res_status_line(unsigned int st_code, const char* text, unsigned int text_len,
                                 struct qbuf* res) {
    qbuf_append(res, HTTP_VERSION_STR, HTTP_VERSION_STR_LEN);

    char code_str[16];
    int code_len = snprintf(code_str, 16, " %u ", st_code);
    qbuf_append(res, code_str, code_len);
    qbuf_append(res, text, text_len);
    qbuf_append(res, "\r\n", 2);
}

int http_response_encode_head(unsigned int status_code,
                              const char* text, unsigned int text_len,
                              const struct http_kv_list* header_list,
                              unsigned long content_len, struct qbuf* res) {
    if (qbuf_reserve(res, qbuf_size(res) + 128 + content_len) != 0) {
        return HRC_NOMEM;
    }

    pack_res_status_line(status_code, text, text_len, res);

    int rc = http_header_encode(header_list, content_len, res);
    if (rc != HRC_OK) {
        return rc;
    }

    return HRC_OK;
}
