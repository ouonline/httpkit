/**
 * references:
 * https://tools.ietf.org/html/rfc2616
 * https://tools.ietf.org/html/rfc3986
 **/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "http_common.h"
#include "http_request.h"
#include "utils/str_utils.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

static inline int parse_query_pair_part(const char* data,
                                        unsigned int len,
                                        struct qbuf* q) {
    qbuf_init(q);
    if (qbuf_resize(q, len) != 0) {
        return HRE_NOMEM;
    }

    int ret = http_decode_url(data, len, q->base, len);
    if (ret >= 0) {
        q->len = ret;
    } else {
        qbuf_destroy(q);
    }

    return ret;
}

static int parse_query_pair(const char* data, unsigned int len,
                            struct http_request_option* opt) {
    int ret;
    unsigned int part_len;
    const char* cursor;

    cursor = (const char*)memmem(data, len, "=", 1);
    if ((!cursor) || (cursor == data)) {
        return HRE_URLDECODE;
    }

    part_len = cursor - data;
    ret = parse_query_pair_part(data, part_len, &opt->key);
    if (ret < 0) {
        return ret;
    }

    part_len = len - part_len - 1;
    if (part_len > 0) {
        ret = parse_query_pair_part(cursor + 1, part_len,
                                    &opt->value);
        if (ret < 0) {
            qbuf_destroy(&opt->key);
            return ret;
        }
    } else {
        qbuf_init(&opt->value);
    }

    return HRE_SUCCESS;
}

static int parse_query(const char* data, unsigned int len,
                       struct list_node* opts) {
    while (len > 0) {
        struct http_request_option* opt;
        const char* cursor = (const char*)memmem(data, len, "&", 1);

        opt = (struct http_request_option*)malloc(sizeof(struct http_request_option));
        if (!opt) {
            return HRE_NOMEM;
        }

        if (cursor) {
            if (cursor == data) {
                ++data;
                --len;
                continue;
            }

            int err = parse_query_pair(data, cursor - data, opt);
            if (err) {
                free(opt);
                return err;
            }

            list_add_prev(&opt->node, opts);

            len -= (cursor - data + 1);
            data = cursor + 1;
        } else {
            int err = parse_query_pair(data, len, opt);
            if (err) {
                free(opt);
                return err;
            }

            list_add_prev(&opt->node, opts);
            break;
        }
    }

    return HRE_SUCCESS;
}

static int parse_method(const char* data, unsigned int len) {
    if (len == 3 && memcmp(data, "GET", len) == 0) {
        return HTTP_REQUEST_METHOD_GET;
    }

    if (len == 4 && memcmp(data, "POST", len) == 0) {
        return HTTP_REQUEST_METHOD_POST;
    }

    return HTTP_REQUEST_METHOD_UNSUPPORTED;
}

static int parse_abs_path(const char* data, unsigned int len,
                          struct qbuf* path) {
    qbuf_init(path);
    if (qbuf_resize(path, len) != 0) {
        return HRE_NOMEM;
    }

    int ret = http_decode_url(data, len, path->base, len);
    if (ret < 0) {
        qbuf_destroy(path);
        return ret;
    }

    path->len = ret;
    return HRE_SUCCESS;
}

/* URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ] */
static int parse_request_uri(const char* data, unsigned int len,
                             struct qbuf* abs_path, struct list_node* opts) {
    int err;
    const char* cursor = (const char*)memmem(data, len, "?", 1);
    if (cursor) {
        err = parse_abs_path(data, cursor - data, abs_path);
        if (err) {
            return err;
        }

        len -= (cursor - data + 1);
        data = cursor + 1;
        cursor = (const char*)memmem(data, len, "#", 1);
        if (cursor) {
            err = parse_query(data, cursor - data, opts);
            if (err) {
                return err;
            }
        } else {
            err = parse_query(data, len, opts);
            if (err) {
                return err;
            }
        }
    } else {
        cursor = (const char*)memmem(data, len, "#", 1);
        if (cursor) {
            err = parse_abs_path(data, cursor - data, abs_path);
            if (err) {
                return err;
            }
        } else {
            err = parse_abs_path(data, len, abs_path);
            if (err) {
                return err;
            }
        }
    }

    return HRE_SUCCESS;
}

/* Method SP Request-URI SP HTTP-Version CRLF */
static int parse_request_line(const char* data, unsigned int len,
                              struct http_request_line* line) {
    const char* cursor;

    cursor = (const char*)memmem(data, len, " ", 1);
    if (!cursor) {
        return HRE_REQLINE;
    }

    line->method = parse_method(data, cursor - data);
    if (line->method == HTTP_REQUEST_METHOD_UNSUPPORTED) {
        return HRE_REQMETHOD;
    }

    len -= (cursor - data + 1);
    data = cursor + 1 /* skip the space */;

    cursor = (const char*)memmem(data, len, " ", 1);
    if (!cursor) {
        return HRE_REQLINE;
    }

    return parse_request_uri(data, cursor - data, &line->abs_path,
                             &line->option_list);
}

typedef void (*parse_header_field_func)(const char* key, unsigned int keylen,
                                        const char* value, unsigned int valuelen,
                                        struct http_request_header* header);

static void get_content_length_func(const char* key, unsigned int keylen,
                                    const char* value, unsigned int valuelen,
                                    struct http_request_header* header) {
    (void)key;
    (void)keylen;

    header->content_len = 0;
    ndec2int(value, valuelen, (int*)(&header->content_len));
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

static void get_content_type_func(const char* key, unsigned int keylen,
                                  const char* value, unsigned int valuelen,
                                  struct http_request_header* header) {
    (void)key;
    (void)keylen;

    int i;
    const char* cursor;

    cursor = (const char*)memmem(value, valuelen, ";", 1);
    if (cursor) {
        valuelen = cursor - value;
    }

    for (i = 0; g_content_type_list[i].key; ++i) {
        struct content_type* ct = &(g_content_type_list[i]);
        if (valuelen != ct->keylen) {
            continue;
        }

        if (memcmp(value, ct->key, valuelen) != 0) {
            continue;
        }

        header->content_type = ct->type;
        return;
    }
}

#define CONTENT_LENGTH_STR "Content-Length"
#define CONTENT_LENGTH_LEN 14

static struct header_field_handler {
    const char* key;
    unsigned int keylen;
    parse_header_field_func func;
} g_header_filed_func_list[] = {
    {CONTENT_LENGTH_STR, CONTENT_LENGTH_LEN, get_content_length_func},
    {"Content-Type", 12, get_content_type_func},
    {NULL, 0, NULL},
};

static int do_parse_header_field(const char* key, unsigned int klen,
                                 const char* value, unsigned int vlen,
                                 struct header_field_handler* handler,
                                 struct http_request_header* header) {
    if (klen != handler->keylen) {
        return HRE_EMPTY;
    }

    if (memcmp(key, handler->key, klen) != 0) {
        return HRE_EMPTY;
    }

    handler->func(key, klen, value, vlen, header);
    return HRE_SUCCESS;
}

static void parse_all_header_fields(const char* key, unsigned int klen,
                                    const char* value, unsigned int vlen,
                                    struct http_request_header* header) {
    int i;
    for (i = 0; g_header_filed_func_list[i].key; ++i) {
        if (do_parse_header_field(key, klen, value, vlen,
                                  &(g_header_filed_func_list[i]),
                                  header) == 0) {
            return;
        }
    }
}

static void parse_header_line(const char* data, unsigned int len,
                              parse_header_field_func parse_func,
                              struct http_request_header* header) {
    unsigned int keylen = 0;
    const char* end = data + len;
    const char* cursor = (const char*)memmem(data, len, ":", 1);
    if (!cursor) {
        return;
    }
    keylen = cursor - data;

    while (1) {
        ++cursor;
        if (cursor >= end) {
            return; /* nothing */
        }
        if (*cursor != ' ') {
            break;
        }
    }

    parse_func(data, keylen, cursor, len - (cursor - data), header);
}

static void parse_header(const char* data, unsigned int len,
                         parse_header_field_func parse_func,
                         struct http_request_header* header) {
    while (len > 0) {
        const char* eol = (const char*)memmem(data, len, "\r\n", 2);
        if (!eol) {
            return;
        }

        parse_header_line(data, eol - data, parse_func, header);

        len -= (eol - data + 2);
        data = eol + 2;
    }
}

static int parse_content(const char* data, unsigned int len,
                         struct qbuf* content) {
    qbuf_init(content);
    if (qbuf_resize(content, len) != 0) {
        return HRE_NOMEM;
    }

    memcpy(content->base, data, len);
    return HRE_SUCCESS;
}

static int parse(const char* data, unsigned int len, struct http_request* req) {
    int err = 0;
    const char *eoh, *cursor;

    cursor = (const char*)memmem(data, len, "\r\n\r\n", 4);
    if (!cursor) {
        return HRE_HEADER;
    }
    eoh = cursor + 2; /* end of header */

    cursor = (const char*)memmem(data, eoh - data, "\r\n", 2);
    cursor += 2;
    err = parse_request_line(data, cursor - data, &req->req_line);
    if (err) {
        return err;
    }

    if (req->req_line.method == HTTP_REQUEST_METHOD_POST) {
        len -= (cursor - data);
        data = cursor;
        parse_header(data, eoh - data, parse_all_header_fields,
                     &req->header);

        len -= (eoh - data + 2);
        data = eoh + 2;
        err = parse_content(data, len, &req->content);
        if (err) {
            return err;
        }

        if (req->header.content_len > req->content.len) {
            return HRE_CONTENT_SIZE;
        }
    }

    return err;
}

static inline void init_request_line(struct http_request_line* line) {
    line->method = HTTP_REQUEST_METHOD_UNSUPPORTED;
    qbuf_init(&line->abs_path);
    list_init(&line->option_list);
}

static inline void init_request_header(struct http_request_header* header) {
    header->content_type = HTTP_CONTENT_TYPE_UNSUPPORTED;
    header->content_len = 0;
}

static inline void destroy_request_line(struct http_request_line* line) {
    struct list_node *cur, *next;

    list_for_each_safe(cur, next, &line->option_list) {
        struct http_request_option* opt;
        opt = list_entry(cur, struct http_request_option, node);
        __list_del(cur);
        qbuf_destroy(&opt->key);
        qbuf_destroy(&opt->value);
        free(opt);
    }
    list_init(&line->option_list);

    qbuf_destroy(&line->abs_path);
}

static inline void destroy_request_header(struct http_request_header* header) {
    init_request_header(header);
}

static void parse_content_length(const char* key, unsigned int klen,
                                 const char* value, unsigned int vlen,
                                 struct http_request_header* header) {
    if (klen != CONTENT_LENGTH_LEN) {
        return;
    }

    if (memcmp(key, CONTENT_LENGTH_STR, CONTENT_LENGTH_LEN) != 0) {
        return;
    }

    get_content_length_func(key, klen, value, vlen, header);
}

/* ------------------------------------------------------------------------- */

void http_request_destroy(struct http_request* req) {
    destroy_request_line(&req->req_line);
    destroy_request_header(&req->header);
    qbuf_destroy(&req->content);
}

int http_request_init(struct http_request* req, const char* data,
                      unsigned int len) {
    int err;

    init_request_line(&req->req_line);
    init_request_header(&req->header);
    qbuf_init(&req->content);

    err = parse(data, len, req);
    if (err) {
        http_request_destroy(req);
    }

    return err;
}

int http_request_get_method(const struct http_request* req) {
    return req->req_line.method;
}

void http_request_get_abs_path(const struct http_request* req,
                               struct qbuf_ref* ref) {
    ref->base = req->req_line.abs_path.base;
    ref->len = req->req_line.abs_path.len;
}

int http_request_for_each_option(const struct http_request* req, void* arg,
                                 int (*f)(void* arg,
                                          const char* k, unsigned int klen,
                                          const char* v, unsigned int vlen)) {
    struct list_node* cur;

    list_for_each(cur, &req->req_line.option_list) {
        int err;
        struct http_request_option* opt;

        opt = list_entry(cur, struct http_request_option, node);
        err = f(arg, opt->key.base, opt->key.len,
                opt->value.base, opt->value.len);
        if (err) {
            return err;
        }
    }

    return HRE_SUCCESS;
}

unsigned int http_request_get_content_type(const struct http_request* req) {
    return req->header.content_type;
}

void http_request_get_content(const struct http_request* req,
                              struct qbuf_ref* ref) {
    ref->base = req->content.base;
    ref->len = req->content.len;
}

int http_get_request_size(const char* data, unsigned int len) {
    const char* cursor = (const char*)memmem(data, len, "\r\n\r\n", 4);
    if (!cursor) {
        return HRE_HEADER;
    }

    struct http_request_header header;
    parse_header(data, cursor + 2 - data, parse_content_length, &header);

    return (cursor - data + 4 + header.content_len);
}

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
