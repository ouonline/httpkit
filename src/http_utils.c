#include "httpkit/http_utils.h"
#include "httpkit/http_retcode.h"
#include "cutils/str_utils.h"
#include <stddef.h> /* NULL */

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
            ++src;
            if (src_end < src + 2) {
                return HRC_URL;
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

static const struct http_response_status g_http_status[] = {
    {100, "100", 3, "Continue", 8},
    {101, "101", 3, "Switching Protocols", 19},

    {200, "200", 3, "OK", 2},
    {201, "201", 3, "Created", 7},
    {202, "202", 3, "Accepted", 8},
    {203, "203", 3, "Non-Authoritative Information", 29},
    {204, "204", 3, "No Content", 10},
    {205, "205", 3, "Reset Content", 13},
    {206, "206", 3, "Partial Content", 15},

    {300, "300", 3, "Multiple Choices", 16},
    {301, "301", 3, "Moved Permanently", 17},
    {302, "302", 3, "Found", 5},
    {303, "303", 3, "See Other", 9},
    {304, "304", 3, "Not Modified", 12},
    {305, "305", 3, "Use Proxy", 9},
    {307, "307", 3, "Temporary Redirect", 18},

    {400, "400", 3, "Bad Request", 11},
    {401, "401", 3, "Unauthorized", 12},
    {402, "402", 3, "Payment Required", 16},
    {403, "403", 3, "Forbidden", 9},
    {404, "404", 3, "Not Found", 9},
    {405, "405", 3, "Method Not Allowed", 18},
    {406, "406", 3, "Not Acceptable", 14},
    {407, "407", 3, "Proxy Authentication Required", 29},
    {408, "408", 3, "Request Time-out", 16},
    {409, "409", 3, "Conflict", 8},
    {410, "410", 3, "Gone", 4},
    {411, "411", 3, "Length Required", 15},
    {412, "412", 3, "Precondition Failed", 19},
    {413, "413", 3, "Request Entity Too Large", 24},
    {414, "414", 3, "Request-URI Too Large", 21},
    {415, "415", 3, "Unsupported Media Type", 22},
    {416, "416", 3, "Requested range not satisfiable", 31},
    {417, "417", 3, "Expectation Failed", 18},

    {500, "500", 3, "Internal Server Error", 21},
    {501, "501", 3, "Not Implemented", 15},
    {502, "502", 3, "Bad Gateway", 11},
    {503, "503", 3, "Service Unavailable", 19},
    {504, "504", 3, "Gateway Time-out", 16},
    {505, "505", 3, "HTTP Version not supported", 26},
};

static const unsigned int g_http_status_num = sizeof(g_http_status) / sizeof(struct http_response_status);

const struct http_response_status* http_response_status_lookup(unsigned int code) {
    unsigned int low = 0, high = g_http_status_num - 1;
    while (low <= high) {
        unsigned int mid = (low + high) / 2;
        unsigned int mid_code = g_http_status[mid].code;
        if (code < mid_code) {
            high = mid - 1;
        } else if (code > mid_code) {
            low = mid + 1;
        } else {
            return &g_http_status[mid];
        }
    }
    return NULL;
}
