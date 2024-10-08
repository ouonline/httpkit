#include "httpkit/http_retcode.h"
#include "http_header_encode.h"

int http_header_encode(struct qbuf* res, const char* key, unsigned int klen,
                       const char* value, unsigned int vlen) {
    int ret = qbuf_reserve(res, qbuf_size(res) + klen + vlen +
                           3 /* ":" and "\r\n" */);
    if (ret != 0) {
        return HRC_NOMEM;
    }

    qbuf_append(res, key, klen);
    qbuf_append_c(res, ':');
    qbuf_append(res, value, vlen);
    qbuf_append(res, "\r\n", 2);

    return HRC_OK;
}
