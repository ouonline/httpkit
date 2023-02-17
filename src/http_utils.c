#include "httpkit/http_utils.h"
#include "httpkit/http_common.h"
#include "cutils/str_utils.h"

int http_url_decode(const char* src, unsigned int src_size, struct qbuf* buf) {
    int err = qbuf_resize(buf, src_size * 3);
    if (err) {
        return HRC_NOMEM;
    }

    char* dst_cursor = qbuf_data(buf);
    const char* src_end = src + src_size;
    const char* dst_end = dst_cursor + qbuf_size(buf);

    while (src < src_end && dst_cursor < dst_end) {
        if (*src == '+') {
            *dst_cursor = ' ';
            ++src;
        } else if (*src == '%') {
            int ch = 0;

            ++src;
            if (src_end < src + 2) {
                return HRC_URLDECODE;
            }

            *dst_cursor = nhex2long(src, 2);
            src += 2;
        } else {
            *dst_cursor = *src;
            ++src;
        }

        ++dst_cursor;
    }

    qbuf_resize(buf, dst_cursor - (char*)qbuf_data(buf));

    return HRC_OK;
}
