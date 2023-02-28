#include "httpkit/http_common.h"
#include "httpkit/http_response_encode.h"
#include "http_header_encode.h"

static const struct status_info {
    unsigned int code;
    const char* code_str;
    unsigned int code_len;
    const char* text_str;
    unsigned int text_len;
} g_http_status[] = {
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
    {0, NULL, 0, NULL, 0},
};

#define HTTP_VERSION_STR "HTTP/1.1"
#define HTTP_VERSION_STR_LEN 8

static const struct status_info* __find_status_info(unsigned int st_code) {
    int i;
    for (i = 0; g_http_status[i].code_str; ++i) {
        if (g_http_status[i].code == st_code) {
            return &g_http_status[i];
        }
    }
    return NULL;
}

static void pack_res_status_line(unsigned int st_code, struct qbuf* res) {
    qbuf_append(res, HTTP_VERSION_STR, HTTP_VERSION_STR_LEN);
    qbuf_append_c(res, ' ');

    const struct status_info* si = __find_status_info(st_code);
    if (si) {
        qbuf_append(res, si->code_str, si->code_len);
        qbuf_append_c(res, ' ');
        qbuf_append(res, si->text_str, si->text_len);
    } else {
        qbuf_append(res, "0 unknowncode", 13);
    }
    qbuf_append(res, "\r\n", 2);
}

int http_response_encode_head(unsigned int status_code, struct http_kv_list* header_list,
                              unsigned long content_len, struct qbuf* res) {
    if (qbuf_reserve(res, qbuf_size(res) + 128 + content_len) != 0) {
        return HRC_NOMEM;
    }

    pack_res_status_line(status_code, res);

    int rc = http_header_encode(header_list, content_len, res);
    if (rc != HRC_OK) {
        return rc;
    }

    return HRC_OK;
}
