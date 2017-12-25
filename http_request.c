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
#include <stdio.h>

/* ------------------------------------------------------------------------- */

static inline int parse_query_pair_part(const char* data,
                                        unsigned int len,
                                        struct qbuf* q) {
    qbuf_init(q);
    if (qbuf_reserve(q, len) != 0) {
        return HRE_NOMEM;
    }

    int ret = http_decode_url(data, len, (char*)qbuf_data(q), len);
    if (ret > 0) {
        qbuf_resize(q, ret);
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
    if (qbuf_reserve(path, len) != 0) {
        return HRE_NOMEM;
    }

    int ret = http_decode_url(data, len, (char*)qbuf_data(path), len);
    if (ret < 0) {
        qbuf_destroy(path);
        return ret;
    }

    qbuf_resize(path, ret);
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

static void parse_content(const char* data, unsigned int len,
                          struct qbuf_ref* content) {
    content->base = data;
    content->size = len;
}

static int parse(struct http_request* req) {
    const char* data = (const char*)qbuf_data(&req->raw_data);
    unsigned int len = qbuf_size(&req->raw_data);
    int err = HRE_SUCCESS;
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

        http_header_decode(&req->header, data, eoh - data);

        len -= (eoh - data + 2);
        data = eoh + 2;

        parse_content(data, len, &req->content);

        if (req->header.content_len > req->content.size) {
            return HRE_CONTENT_SIZE;
        }
    }

    return HRE_SUCCESS;
}

static inline void init_request_line(struct http_request_line* line) {
    line->method = HTTP_REQUEST_METHOD_UNSUPPORTED;
    qbuf_init(&line->abs_path);
    list_init(&line->option_list);
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

/* ------------------------------------------------------------------------- */

void http_request_destroy(struct http_request* req) {
    destroy_request_line(&req->req_line);
    http_header_destroy(&req->header);
    qbuf_ref_destroy(&req->content);
    qbuf_destroy(&req->raw_data);
}

int http_request_init(struct http_request* req) {
    init_request_line(&req->req_line);
    http_header_init(&req->header);
    qbuf_ref_init(&req->content);
    qbuf_init(&req->raw_data);
    return 0;
}

int http_request_decode(struct http_request* req, const char* data,
                        unsigned int len) {
    if (qbuf_assign(&req->raw_data, data, len) != 0) {
        return HRE_NOMEM;
    }

    int err = parse(req);
    if (err) {
        http_request_destroy(req);
    }

    return err;
}

static const struct qbuf_ref g_req_method_str[] = {
    {"GET", 3},
    {"POST", 4},
    {NULL, 0},
};

static inline struct http_request_option*
create_option(const char* key, unsigned int klen, const char* value, unsigned int vlen) {
    struct http_request_option* opt =
        (struct http_request_option*)malloc(sizeof(struct http_request_option));
    if (!opt) {
        return NULL;
    }

    if (qbuf_reserve(&opt->key, klen) != 0) {
        goto err_key;
    }

    if (qbuf_reserve(&opt->value, vlen) != 0) {
        goto err_value;
    }

    qbuf_assign(&opt->key, key, klen);
    qbuf_assign(&opt->value, value, vlen);
    return opt;

err_value:
    qbuf_destroy(&opt->key);
err_key:
    free(opt);
    return NULL;
}

/* make sure opt_list is not empty */
static inline void append_option_list(struct qbuf* q, struct list_node* opt_list) {
    struct list_node* cur;
    struct http_request_option* opt;

    cur = list_first(opt_list);
    opt = list_entry(cur, struct http_request_option, node);
    qbuf_append(q, "?", 1);
    qbuf_append(q, qbuf_data(&opt->key), qbuf_size(&opt->key));
    qbuf_append(q, "=", 1);
    qbuf_append(q, qbuf_data(&opt->value), qbuf_size(&opt->value));

    list_for_each_from(cur, list_next(cur), opt_list) {
        opt = list_entry(cur, struct http_request_option, node);
        qbuf_append(q, "&", 1);
        qbuf_append(q, qbuf_data(&opt->key), qbuf_size(&opt->key));
        qbuf_append(q, "=", 1);
        qbuf_append(q, qbuf_data(&opt->value), qbuf_size(&opt->value));
    }
}

static inline void append_req_line(struct qbuf* data,
                                   struct http_request_line* req_line) {
    qbuf_append(data, g_req_method_str[req_line->method].base,
                g_req_method_str[req_line->method].size);

    qbuf_append(data, " ", 1);

    if (qbuf_empty(&req_line->abs_path)) {
        qbuf_append(data, "/", 1);
        qbuf_assign(&req_line->abs_path, "/", 1);
    } else {
        qbuf_append(data, (const char*)qbuf_data(&req_line->abs_path),
                    qbuf_size(&req_line->abs_path));
    }

    if (!list_empty(&req_line->option_list)) {
        append_option_list(data, &req_line->option_list);
    }

    qbuf_append(data, " HTTP/1.1\r\n", 11);
}

static inline void append_req_header(struct qbuf* data,
                                     struct http_header* header,
                                     unsigned int content_len) {
    char tmp[64];
    unsigned int len = sprintf(tmp, "Content-Length: %u\r\n", content_len);
    qbuf_append(data, tmp, len);

    header->content_len = content_len;
}

int http_request_encode(struct http_request* req,
                        const char* content, unsigned int content_len) {
    if (req->req_line.method >= HTTP_REQUEST_METHOD_UNSUPPORTED) {
        return HRE_REQMETHOD;
    }

    qbuf_clear(&req->raw_data);
    if (qbuf_reserve(&req->raw_data, 128 + content_len) != 0) {
        return HRE_NOMEM;
    }

    append_req_line(&req->raw_data, &req->req_line);
    append_req_header(&req->raw_data, &req->header, content_len);
    qbuf_append(&req->raw_data, "\r\n", 2);

    unsigned int content_pos = qbuf_size(&req->raw_data);
    if (content_len > 0) {
        qbuf_append(&req->raw_data, content, content_len);
        req->content.base = (const char*)qbuf_data(&req->raw_data) + content_pos;
        req->content.size = content_len;
    } else {
        req->content.base = NULL;
        req->content.size = 0;
    }

    return HRE_SUCCESS;
}

unsigned int http_request_get_method(const struct http_request* req) {
    return req->req_line.method;
}

void http_request_set_method(struct http_request* req, unsigned int method) {
    req->req_line.method = method;
}

int http_request_set_abs_path(struct http_request* req, const char* data,
                              unsigned int len) {
    if (qbuf_assign(&req->req_line.abs_path, data, len) != 0) {
        return HRE_NOMEM;
    }

    return HRE_SUCCESS;
}

void http_request_get_abs_path(const struct http_request* req,
                               struct qbuf_ref* ref) {
    ref->base = (const char*)qbuf_data(&req->req_line.abs_path);
    ref->size = qbuf_size(&req->req_line.abs_path);
}

int http_request_append_option(struct http_request* req,
                               const char* key, unsigned int klen,
                               const char* value, unsigned int vlen) {
    struct http_request_option* opt;
    opt = (struct http_request_option*)malloc(sizeof(struct http_request_option));
    if (!opt) {
        return HRE_NOMEM;
    }

    if (!qbuf_append(&opt->key, key, klen)) {
        goto err_key;
    }

    if (!qbuf_append(&opt->value, value, vlen)) {
        goto err_value;
    }

    list_add_prev(&req->req_line.option_list, &opt->node);

    return HRE_SUCCESS;

err_value:
    qbuf_destroy(&opt->key);
err_key:
    free(opt);
    return HRE_NOMEM;
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
        err = f(arg, (const char*)qbuf_data(&opt->key), qbuf_size(&opt->key),
                (const char*)qbuf_data(&opt->value), qbuf_size(&opt->value));
        if (err) {
            return err;
        }
    }

    return HRE_SUCCESS;
}

void http_request_get_content(const struct http_request* req,
                              struct qbuf_ref* ref) {
    ref->base = req->content.base;
    ref->size = req->content.size;
}

void http_request_get_packet(const struct http_request* req,
                             struct qbuf_ref* ref) {
    ref->base = (const char*)qbuf_data(&req->raw_data);
    ref->size = qbuf_size(&req->raw_data);
}

int http_get_request_size(const char* data, unsigned int len) {
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
