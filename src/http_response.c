#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "httpkit/http_common.h"
#include "httpkit/http_response.h"
#include <string.h>
#include <stdio.h>

static const struct status_info {
    unsigned int code;
    const char* code_str;
    unsigned int code_len;
    const char* text_str;
    unsigned int text_len;
} g_http_status[] = {
    {HTTP_STATUS_100, "100", 3, "Continue", 8},
    {HTTP_STATUS_101, "101", 3, "Switching Protocols", 19},

    {HTTP_STATUS_200, "200", 3, "OK", 2},
    {HTTP_STATUS_201, "201", 3, "Created", 7},
    {HTTP_STATUS_202, "202", 3, "Accepted", 8},
    {HTTP_STATUS_203, "203", 3, "Non-Authoritative Information", 29},
    {HTTP_STATUS_204, "204", 3, "No Content", 10},
    {HTTP_STATUS_205, "205", 3, "Reset Content", 13},
    {HTTP_STATUS_206, "206", 3, "Partial Content", 15},

    {HTTP_STATUS_300, "300", 3, "Multiple Choices", 16},
    {HTTP_STATUS_301, "301", 3, "Moved Permanently", 17},
    {HTTP_STATUS_302, "302", 3, "Found", 5},
    {HTTP_STATUS_303, "303", 3, "See Other", 9},
    {HTTP_STATUS_304, "304", 3, "Not Modified", 12},
    {HTTP_STATUS_305, "305", 3, "Use Proxy", 9},
    {HTTP_STATUS_307, "307", 3, "Temporary Redirect", 18},

    {HTTP_STATUS_400, "400", 3, "Bad Request", 11},
    {HTTP_STATUS_401, "401", 3, "Unauthorized", 12},
    {HTTP_STATUS_402, "402", 3, "Payment Required", 16},
    {HTTP_STATUS_403, "403", 3, "Forbidden", 9},
    {HTTP_STATUS_404, "404", 3, "Not Found", 9},
    {HTTP_STATUS_405, "405", 3, "Method Not Allowed", 18},
    {HTTP_STATUS_406, "406", 3, "Not Acceptable", 14},
    {HTTP_STATUS_407, "407", 3, "Proxy Authentication Required", 29},
    {HTTP_STATUS_408, "408", 3, "Request Time-out", 16},
    {HTTP_STATUS_409, "409", 3, "Conflict", 8},
    {HTTP_STATUS_410, "410", 3, "Gone", 4},
    {HTTP_STATUS_411, "411", 3, "Length Required", 15},
    {HTTP_STATUS_412, "412", 3, "Precondition Failed", 19},
    {HTTP_STATUS_413, "413", 3, "Request Entity Too Large", 24},
    {HTTP_STATUS_414, "414", 3, "Request-URI Too Large", 21},
    {HTTP_STATUS_415, "415", 3, "Unsupported Media Type", 22},
    {HTTP_STATUS_416, "416", 3, "Requested range not satisfiable", 31},
    {HTTP_STATUS_417, "417", 3, "Expectation Failed", 18},

    {HTTP_STATUS_500, "500", 3, "Internal Server Error", 21},
    {HTTP_STATUS_501, "501", 3, "Not Implemented", 15},
    {HTTP_STATUS_502, "502", 3, "Bad Gateway", 11},
    {HTTP_STATUS_503, "503", 3, "Service Unavailable", 19},
    {HTTP_STATUS_504, "504", 3, "Gateway Time-out", 16},
    {HTTP_STATUS_505, "505", 3, "HTTP Version not supported", 26},
    {HTTP_STATUS_UNSUPPORTED, NULL, 0, NULL, 0},
};

static inline void init_http_response_status(struct http_response_status* st) {
    st->code = HTTP_STATUS_UNSUPPORTED;
    qbuf_ref_init(&st->text);
    qbuf_ref_init(&st->http_version);
}

static inline void destroy_http_response_status(struct http_response_status* st) {
    st->code = HTTP_STATUS_UNSUPPORTED;
    qbuf_ref_destroy(&st->text);
    qbuf_ref_destroy(&st->http_version);
}

int http_response_init(struct http_response* res) {
    init_http_response_status(&res->status_line);
    http_header_init(&res->header);
    qbuf_ref_init(&res->content);
    qbuf_init(&res->raw_data);
    return 0;
}

void http_response_destroy(struct http_response* res) {
    destroy_http_response_status(&res->status_line);
    http_header_destroy(&res->header);
    qbuf_ref_destroy(&res->content);
    qbuf_destroy(&res->raw_data);
}

unsigned int http_response_get_status_code(const struct http_response* res) {
    return res->status_line.code;
}

void http_response_get_content(const struct http_response* res,
                               struct qbuf_ref* ref) {
    ref->base = res->content.base;
    ref->size = res->content.size;
}

#define HTTP_VERSION_STR "HTTP/1.1"
#define HTTP_VERSION_STR_LEN 8

static inline void pack_res_status_line(struct http_response* res,
                                        unsigned int st_code) {
    qbuf_append(&res->raw_data, HTTP_VERSION_STR, HTTP_VERSION_STR_LEN);
    qbuf_append(&res->raw_data, " ", 1);
    qbuf_append(&res->raw_data, g_http_status[st_code].code_str,
                g_http_status[st_code].code_len);
    qbuf_append(&res->raw_data, " ", 1);
    qbuf_append(&res->raw_data, g_http_status[st_code].text_str,
                g_http_status[st_code].text_len);
    qbuf_append(&res->raw_data, "\r\n", 2);

    res->status_line.code = st_code;
    res->status_line.text.base = g_http_status[st_code].code_str;
    res->status_line.text.size = g_http_status[st_code].code_len;
    res->status_line.http_version.base = HTTP_VERSION_STR;
    res->status_line.http_version.size = HTTP_VERSION_STR_LEN;
}

static inline void pack_res_header(struct http_response* res,
                                   unsigned int content_len) {
    char tmp[64];
    int tmp_len = sprintf(tmp, "Content-Length: %u\r\n\r\n",
                          content_len);
    qbuf_append(&res->raw_data, tmp, tmp_len);

    res->header.content_len = content_len;
}

static inline void pack_res_content(struct http_response* res,
                                    const char* content, unsigned int content_len) {
    unsigned int content_offset = qbuf_size(&res->raw_data);
    qbuf_append(&res->raw_data, content, content_len);

    res->content.base = (const char*)qbuf_data(&res->raw_data) + content_offset;
    res->content.size = content_len;
}

int http_response_encode(struct http_response* res, unsigned int status_code,
                         const char* content, unsigned int content_len) {
    if (status_code >= HTTP_STATUS_UNSUPPORTED) {
        return HRE_HTTP_STATUS;
    }

    qbuf_clear(&res->raw_data);
    if (qbuf_reserve(&res->raw_data, 128 + content_len) != 0) {
        return HRE_NOMEM;
    }

    pack_res_status_line(res, status_code);
    pack_res_header(res, content_len);

    if (content_len > 0 && content) {
        pack_res_content(res, content, content_len);
    } else {
        res->content.base = NULL;
        res->content.size = 0;
    }

    return 0;
}

static inline const struct status_info* find_status_info(const char* code_str) {
    for (int i = 0; g_http_status[i].code_str; ++i) {
        if (memcmp(code_str, g_http_status[i].code_str, 3) != 0) {
            return &g_http_status[i];
        }
    }

    return NULL;
}

static int parse_status_line(const char* data, unsigned int len,
                             struct http_response_status* st_line) {
    const char* cursor = (const char*)memmem(data, len, " ", 1);
    if (!cursor) {
        return HRE_RESLINE;
    }

    st_line->http_version.base = data;
    st_line->http_version.size = cursor - data;

    len -= (cursor - data + 1);
    data = cursor + 1; /* skip space */

    cursor = (const char*)memmem(data, len, " ", 1);
    if (!cursor) {
        return HRE_RESLINE;
    }
    if (cursor - data != 3) { /* 3 digits */
        return HRE_RESLINE;
    }

    const struct status_info* si = find_status_info(cursor);
    if (!si) {
        return HRE_RESLINE;
    }
    st_line->code = si->code;

    len -= (cursor - data + 1);
    data = cursor + 1;

    st_line->text.base = data;
    st_line->text.size = len;

    return HRE_SUCCESS;
}

static inline void parse_content(const char* data, unsigned int len,
                                 struct qbuf_ref* content) {
    content->base = data;
    content->size = len;
}

int http_response_decode(struct http_response* res, const char* data,
                         unsigned int len) {
    if (qbuf_assign(&res->raw_data, data, len) != 0) {
        return HRE_NOMEM;
    }
    data = (const char*)qbuf_data(&res->raw_data);

    int err = HRE_SUCCESS;
    const char *eoh, *cursor;

    cursor = (const char*)memmem(data, len, "\r\n\r\n", 4);
    if (!cursor) {
        return HRE_HEADER;
    }
    eoh = cursor + 2; /* end of header */

    cursor = (const char*)memmem(data, eoh - data, "\r\n", 2);
    cursor += 2;
    err = parse_status_line(data, cursor - data, &res->status_line);
    if (err) {
        return err;
    }

    http_header_decode(&res->header, cursor, eoh - cursor);

    cursor = eoh + 2; /* skip ending mark: \r\n */
    parse_content(cursor, len - (cursor - data), &res->content);

    return HRE_SUCCESS;
}

void http_response_get_packet(const struct http_response* res,
                              struct qbuf_ref* pkt) {
    pkt->base = (const char*)qbuf_data(&res->raw_data);
    pkt->size = qbuf_size(&res->raw_data);
}

int http_get_response_size(const char* data, unsigned int len) {
    unsigned int ret = 0;
    const char* cursor = (const char*)memmem(data, len, "\r\n\r\n", 4);
    if (!cursor) {
        return HRE_HEADER;
    }

    struct http_header header;
    http_header_init(&header);
    http_header_decode(&header, data, cursor + 2 - data);
    ret = (cursor - data + 4 + header.content_len);
    http_header_destroy(&header);

    return ret;
}
