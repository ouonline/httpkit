#include "http_utils.h"
#include "http_common.h"
#include "utils/str_utils.h"

int http_decode_url(const char* src, unsigned int src_size,
                    char* dst, unsigned int dst_size) {
    char* dst_cursor = dst;
    const char* src_end = src + src_size;
    const char* dst_end = dst + dst_size;

    while (src < src_end && dst_cursor < dst_end) {
        if (*src == '+') {
            *dst_cursor = ' ';
            ++src;
        } else if (*src == '%') {
            int ch = 0;

            ++src;
            if (src_end < src + 2) {
                return HRE_URLDECODE;
            }

            if (nhex2int(src, 2, &ch) != 0) {
                return HRE_URLDECODE;
            }

            *dst_cursor = ch;
            src += 2;
        } else {
            *dst_cursor = *src;
            ++src;
        }

        ++dst_cursor;
    }

    return (dst_cursor - dst);
}
