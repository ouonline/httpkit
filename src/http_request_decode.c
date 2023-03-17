/**
 * references:
 * https://tools.ietf.org/html/rfc2616
 * https://tools.ietf.org/html/rfc3986
 **/

#include "httpkit/http_common.h"
#include "httpkit/http_request_decode.h"
#include "http_header_decode.h"
#include "misc.h"
#include "cutils/str_utils.h" /* memmem() */
#include <stdlib.h>

/* ------------------------------------------------------------------------- */

static void __request_line_init(struct http_request_line* line) {
    qbuf_ol_reset(&line->method);
    qbuf_ol_reset(&line->abs_path);
    http_kv_ol_list_init(&line->option_list);
}

static void __reqeust_line_destroy(struct http_request_line* line) {
    qbuf_ol_reset(&line->method);
    qbuf_ol_reset(&line->abs_path);
    http_kv_ol_list_destroy(&line->option_list);
}

static int __parse_query_pair(const char* base, const char* data, unsigned long len,
                              struct qbuf_ol* key, struct qbuf_ol* value) {
    const char* cursor = memmem(data, len, "=", 1);
    if ((!cursor) || (cursor == data)) {
        return HRC_REQOPTION;
    }

    key->off = data - base;
    key->len = cursor - data;

    len -= (key->len + 1);
    if (len > 0) {
        value->off = cursor + 1 - base;
        value->len = len;
    } else {
        value->off = 0;
        value->len = 0;
    }

    return HRC_OK;
}

static int __parse_query(const char* base, const char* data, unsigned long len,
                         struct http_kv_ol_list* opts) {
    while (len > 0) {
        struct qbuf_ol key, value;

        const char* cursor = (const char*)memmem(data, len, "&", 1);
        if (cursor) {
            if (cursor == data) {
                ++data;
                --len;
                continue;
            }

            int rc = __parse_query_pair(base, data, cursor - data, &key, &value);
            if (rc != HRC_OK) {
                return rc;
            }

            rc = http_kv_ol_list_update(opts, base, key.off, key.len, value.off, value.len);
            if (rc != HRC_OK) {
                return rc;
            }

            len -= (cursor - data + 1);
            data = cursor + 1;
        } else {
            int rc = __parse_query_pair(base, data, len, &key, &value);
            if (rc != HRC_OK) {
                return rc;
            }

            rc = http_kv_ol_list_update(opts, base, key.off, key.len, value.off, value.len);
            if (rc != HRC_OK) {
                return rc;
            }

            break;
        }
    }

    return HRC_OK;
}

/* URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ] */
static int __parse_request_uri(const char* base, const char* data, unsigned long len,
                               struct qbuf_ol* abs_path, struct http_kv_ol_list* opts) {
    int rc;

    const char* cursor = memmem(data, len, "?", 1);
    if (cursor) {
        abs_path->off = data - base;
        abs_path->len = cursor - data;

        len -= (cursor - data + 1);
        data = cursor + 1;

        cursor = memmem(data, len, "#", 1);
        if (cursor) {
            rc = __parse_query(base, data, cursor - data, opts);
            if (rc != HRC_OK) {
                return rc;
            }
        } else {
            rc = __parse_query(base, data, len, opts);
            if (rc != HRC_OK) {
                return rc;
            }
        }
    } else {
        abs_path->off = data - base;
        cursor = memmem(data, len, "#", 1);
        if (cursor) {
            abs_path->len = cursor - data;
        } else {
            abs_path->len = len;
        }
    }

    return HRC_OK;
}

/* Method SP Request-URI SP HTTP-Version CRLF */
static int __request_line_decode(struct http_request_line* l, const char* data,
                                 unsigned int len) {
    const char* base = data;
    const char* cursor;

    /* ----- method ----- */

    cursor = memmem(data, len, " ", 1);
    if (!cursor) {
        return HRC_REQLINE;
    }

    l->method.off = 0;
    l->method.len = cursor - data;

    len -= (cursor - data + 1);
    data = cursor + 1 /* skip the space */;

    /* ----- uri ----- */

    cursor = memmem(data, len, " ", 1);
    if (!cursor) {
        return HRC_REQLINE;
    }

    return __parse_request_uri(base, data, cursor - data, &l->abs_path, &l->option_list);
}

/* ------------------------------------------------------------------------- */

void http_request_decode_context_init(struct http_request_decode_context* ctx) {
    ctx->state = HTTP_REQ_EXPECT_REQLINE;
    ctx->base = NULL;
    ctx->offset = 0;
    __request_line_init(&ctx->req_line);
    http_kv_ol_list_init(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
}

void http_request_decode_context_destroy(struct http_request_decode_context* ctx) {
    ctx->state = HTTP_REQ_EXPECT_REQLINE;
    ctx->base = NULL;
    ctx->offset = 0;
    __reqeust_line_destroy(&ctx->req_line);
    http_kv_ol_list_destroy(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
}

int http_request_decode(struct http_request_decode_context* ctx, const char* data,
                        unsigned long len) {
    ctx->base = data; /* update base addr */
    if (ctx->state == HTTP_REQ_EXPECT_END) {
        return HRC_OK;
    }

    data += ctx->offset;
    len -= ctx->offset;

    switch (ctx->state) {
        case HTTP_REQ_EXPECT_REQLINE: {
            const char* cursor = memmem(data, len, "\r\n", 2);
            if (!cursor) {
                return HRC_MORE_DATA;
            }
            if (cursor == data) {
                return HRC_REQLINE;
            }

            int rc = __request_line_decode(&ctx->req_line, data, cursor - data);
            if (rc != HRC_OK) {
                return rc;
            }

            len -= (cursor + 2 - data);
            ctx->offset += (cursor + 2 - data);
            data = cursor + 2 /* skip '\r\n' */;
            ctx->state = HTTP_REQ_EXPECT_HEADER;
        }
        case HTTP_REQ_EXPECT_HEADER: {
            unsigned long offset_before = ctx->offset;
            int rc = http_header_decode(data, len, ctx->base, &ctx->header_list,
                                        &ctx->offset);
            if (rc != HRC_OK) {
                return rc;
            }

            len -= (ctx->offset - offset_before);
            set_content_len(ctx->base, &ctx->header_list, &ctx->content_length);
            ctx->state = HTTP_REQ_EXPECT_CONTENT;
        }
        case HTTP_REQ_EXPECT_CONTENT: {
            if (len < ctx->content_length) {
                return HRC_MORE_DATA;
            }
            ctx->content_offset = ctx->offset;
            ctx->offset += ctx->content_length;
            ctx->state = HTTP_REQ_EXPECT_END;
        }
    }

    return HRC_OK;
}

void http_request_get_method(const struct http_request_decode_context* ctx, struct qbuf_ref* res) {
    res->base = ctx->base + ctx->req_line.method.off;
    res->size = ctx->req_line.method.len;
}

void http_request_get_abs_path(const struct http_request_decode_context* ctx, struct qbuf_ref* res) {
    res->base = ctx->base + ctx->req_line.abs_path.off;
    res->size = ctx->req_line.abs_path.len;
}

void http_request_get_option(const struct http_request_decode_context* ctx, const char* key,
                             unsigned int klen, struct qbuf_ref* value) {
    struct qbuf_ol* v = http_kv_ol_list_get(&ctx->req_line.option_list, ctx->base, key, klen);
    if (v) {
        value->base = ctx->base + v->off;
        value->size = v->len;
    } else {
        value->base = NULL;
        value->size = 0;
    }
}

int http_request_for_each_option(const struct http_request_decode_context* ctx, void* arg,
                                 int (*f)(void* arg,
                                          const char* k, unsigned int klen,
                                          const char* v, unsigned int vlen)) {
    return http_kv_ol_list_for_each(&ctx->req_line.option_list, ctx->base, arg, f);
}

void http_request_get_header(const struct http_request_decode_context* ctx, const char* key,
                             unsigned int klen, struct qbuf_ref* value) {
    struct qbuf_ol* v = http_kv_ol_list_get(&ctx->header_list, ctx->base, key, klen);
    if (v) {
        value->base = ctx->base + v->off;
        value->size = v->len;
    } else {
        value->base = NULL;
        value->size = 0;
    }
}

int http_request_for_each_header(const struct http_request_decode_context* ctx, void* arg,
                                 int (*f)(void* arg,
                                          const char* k, unsigned int klen,
                                          const char* v, unsigned int vlen)) {
    return http_kv_ol_list_for_each(&ctx->header_list, ctx->base, arg, f);
}

void http_request_get_content(const struct http_request_decode_context* ctx,
                              struct qbuf_ref* res) {
    if (ctx->state == HTTP_REQ_EXPECT_END) {
        res->base = ctx->base + ctx->content_offset;
        res->size = ctx->content_length;
    } else {
        res->base = NULL;
        res->size = 0;
    }
}

unsigned long http_request_get_size(const struct http_request_decode_context* ctx) {
    if (ctx->state == HTTP_REQ_EXPECT_END) {
        return ctx->offset;
    }
    return 0;
}
