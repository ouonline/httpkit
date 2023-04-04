#include "httpkit/http_request_encode.h"
#include "http_header_encode.h"
#include "def.h"

int http_request_encode_request_line(struct qbuf* res, const char* method, unsigned int method_len,
                                     const char* url, unsigned int url_len) {
    int ret = qbuf_reserve(res, qbuf_size(res) + method_len + url_len + HTTP_VERSION_STR_LEN +
                           4 /* 2 spaces and "\r\n" */);
    if (ret != 0) {
        return HRC_NOMEM;
    }

    qbuf_append(res, method, method_len);
    qbuf_append_c(res, ' ');
    qbuf_append(res, url, url_len);
    qbuf_append(res, " " HTTP_VERSION_STR "\r\n", HTTP_VERSION_STR_LEN + 3);

    return HRC_OK;
}

int http_request_encode_header(struct qbuf* res, const char* key, unsigned int klen,
                               const char* value, unsigned int vlen) {
    return http_header_encode(res, key, klen, value, vlen);
}
