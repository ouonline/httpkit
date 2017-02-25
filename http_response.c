#include "http_common.h"
#include "http_response.h"
#include <string.h>
#include <stdio.h>

static struct qbuf_ref g_http_status_code[] = {
    {"100 Continue", 12},
    {"101 Switching Protocols", 23},

    {"200 OK", 6},
    {"201 Created", 11},
    {"202 Accepted", 12},
    {"203 Non-Authoritative Information", 33},
    {"204 No Content", 14},
    {"205 Reset Content", 17},
    {"206 Partial Content", 19},

    {"300 Multiple Choices", 20},
    {"301 Moved Permanently", 21},
    {"302 Found", 9},
    {"303 See Other", 13},
    {"304 Not Modified", 16},
    {"305 Use Proxy", 13},
    {"307 Temporary Redirect", 22},

    {"400 Bad Request", 15},
    {"401 Unauthorized", 16},
    {"402 Payment Required", 20},
    {"403 Forbidden", 13},
    {"404 Not Found", 13},
    {"405 Method Not Allowed", 22},
    {"406 Not Acceptable", 18},
    {"407 Proxy Authentication Required", 33},
    {"408 Request Time-out", 20},
    {"409 Conflict", 12},
    {"410 Gone", 8},
    {"411 Length Required", 19},
    {"412 Precondition Failed", 23},
    {"413 Request Entity Too Large", 28},
    {"414 Request-URI Too Large", 25},
    {"415 Unsupported Media Type", 26},
    {"416 Requested range not satisfiable", 35},
    {"417 Expectation Failed", 22},

    {"500 Internal Server Error", 25},
    {"501 Not Implemented", 19},
    {"502 Bad Gateway", 15},
    {"503 Service Unavailable", 23},
    {"504 Gateway Time-out", 20},
    {"505 HTTP Version not supported", 30},
};

#define RES_STATE              "HTTP/1.1 "
#define RES_CONTENT_LENGTH     "\r\nServer: ohttpd/0.0.1\r\nContent-Length: "
#define RES_CONTENT_TYPE       "\r\nContent-Type: "
#define RES_HEADER_END         "\r\n\r\n"
#define RES_END                "\r\n"
#define RES_STATE_LEN          9
#define RES_CONTENT_LENGTH_LEN 39
#define RES_CONTENT_TYPE_LEN   16
#define RES_HEADER_END_LEN     4
#define RES_END_LEN            2
#define RES_HEADER_PRELEN      (RES_STATE_LEN + RES_CONTENT_LENGTH_LEN + \
                                RES_CONTENT_TYPE_LEN + RES_HEADER_END_LEN + \
                                RES_END_LEN)

int http_response_init(struct http_response* res) {
    qbuf_init(&res->buf);
    return 0;
}

void http_response_destroy(struct http_response* res) {
    qbuf_destroy(&res->buf);
}

void http_response_get_data(struct http_response* res, struct qbuf_ref* ref) {
    ref->base = res->buf.base;
    ref->len = res->buf.len;
}

static struct content_type {
    const char* key;
    unsigned int keylen;
    unsigned int type;
} g_content_type_list[] = {
    {"text/plain", 10, HTTP_CONTENT_TYPE_PLAIN},
    {"application/json", 16, HTTP_CONTENT_TYPE_JSON},
    {"application/x-www-form-urlencoded", 33, HTTP_CONTENT_TYPE_FORM},
    {"text/xml", 8, HTTP_CONTENT_TYPE_XML},
    {"text/html", 9, HTTP_CONTENT_TYPE_HTML},
    {NULL, 0, HTTP_CONTENT_TYPE_UNSUPPORTED},
};

static int do_pack_response(unsigned int status_code, unsigned int content_type,
                            const char* content, unsigned int content_len,
                            char* buf) {
    char* cursor = buf;
    struct qbuf_ref* status = &g_http_status_code[status_code];
    struct content_type* ct = &g_content_type_list[content_type];

    memcpy(buf, RES_STATE, RES_STATE_LEN);
    cursor += RES_STATE_LEN;
    memcpy(cursor, status->base, status->len);
    cursor += status->len;

    memcpy(cursor, RES_CONTENT_LENGTH, RES_CONTENT_LENGTH_LEN);
    cursor += RES_CONTENT_LENGTH_LEN;
    cursor += sprintf(cursor, "%u", content_len);

    memcpy(cursor, RES_CONTENT_TYPE, RES_CONTENT_TYPE_LEN);
    cursor += RES_CONTENT_TYPE_LEN;
    memcpy(cursor, ct->key, ct->keylen);
    cursor += ct->keylen;

    memcpy(cursor, RES_HEADER_END, RES_HEADER_END_LEN);
    cursor += RES_HEADER_END_LEN;

    memcpy(cursor, content, content_len);
    cursor += content_len;

    memcpy(cursor, RES_HEADER_END, RES_HEADER_END_LEN);
    cursor += RES_HEADER_END_LEN;

    return (cursor - buf);
}

static int get_response_max_size(unsigned int status_code,
                                 unsigned int content_type,
                                 unsigned int content_len) {
    if (status_code >= HTTP_STATUS_MAX) {
        return HRE_HTTP_STATUS;
    }

    if (content_type >= HTTP_CONTENT_TYPE_UNSUPPORTED) {
        return HRE_CONTENT_TYPE;
    }

    return (RES_HEADER_PRELEN
            + g_http_status_code[status_code].len
            + g_content_type_list[content_type].keylen
            + 10 /* max len of str(UINT_MAX) */ + content_len);
}

int http_response_pack(struct http_response* res, unsigned int status_code,
                       unsigned int content_type,
                       const char* content, unsigned int content_len) {
    int datalen = get_response_max_size(status_code, content_type, content_len);
    if (datalen <= 0) {
        return datalen;
    }

    if (qbuf_resize(&res->buf, datalen) != 0) {
        return HRE_NOMEM;
    }

    datalen = do_pack_response(status_code, content_type, content, content_len, res->buf.base);
    if (datalen >= 0) {
        qbuf_resize(&res->buf, datalen);
        return 0;
    }

    return datalen;
}
