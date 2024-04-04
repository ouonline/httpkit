/**
 * references:
 * https://tools.ietf.org/html/rfc2616
 * https://tools.ietf.org/html/rfc3986
 **/

#include "httpkit/http_retcode.h"
#include "httpkit/http_request_decode.h"
#include "http_header_decode.h"
#include "misc.h"

/* ------------------------------------------------------------------------- */

static void __request_line_init(struct http_request_line* line) {
    http_item_reset(&line->method);
    http_item_reset(&line->abs_path);
    http_item_reset(&line->fragment);
    http_item_reset(&line->version);
    http_kv_list_init(&line->query_list);
}

static void __reqeust_line_destroy(struct http_request_line* line) {
    http_item_reset(&line->method);
    http_item_reset(&line->abs_path);
    http_item_reset(&line->fragment);
    http_item_reset(&line->version);
    http_kv_list_destroy(&line->query_list);
}

static int __request_line_decode_method(const char* base, unsigned long len,
                                        struct http_item* method, unsigned long* offset) {
    const char* cursor = base;
    const char* end = base + len;

    method->off = 0;
    while (*cursor != ' ') {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
    }
    method->len = cursor - base;

    *offset += (method->len + 1);
    return HRC_OK;
}

/*
  Request-Line = Method SP Request-URI SP HTTP-Version CRLF
  Request-URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
*/
static int __request_line_decode_others(const char* base, const char* cursor, unsigned long len,
                                        struct http_request_line* l, int* state, unsigned long* offset) {
    const char* end = cursor + len;

    switch (*state) {
        case HTTP_REQ_EXPECT_ABS_PATH: {
            l->abs_path.off = cursor - base;
            while (1) {
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
                if (*cursor == ' ') {
                    l->abs_path.len = cursor - base - l->abs_path.off;
                    (*offset) += l->abs_path.len + 1;
                    *state = HTTP_REQ_EXPECT_VERSION;

                    ++cursor; /* skips ' ' */
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_VERSION;
                }
                if (*cursor == '?') {
                    l->abs_path.len = cursor - base - l->abs_path.off;
                    (*offset) += l->abs_path.len + 1;
                    *state = HTTP_REQ_EXPECT_QUERY;

                    ++cursor; /* skips '?' */
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_QUERY;
                }
                if (*cursor == '#') {
                    l->abs_path.len = cursor - base - l->abs_path.off;
                    (*offset) += l->abs_path.len + 1;
                    *state = HTTP_REQ_EXPECT_FRAGMENT;

                    ++cursor; /* skips '#' */
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_FRAGMENT;
                }
            }
        }
        HTTP_REQ_EXPECT_QUERY:
        case HTTP_REQ_EXPECT_QUERY: {
            int rc;
            const char* last_pos = cursor;
            struct http_item key, value;

            if (*cursor == ' ') {
                ++cursor; /* skips ' ' */
                ++(*offset);
                *state = HTTP_REQ_EXPECT_VERSION;
                goto HTTP_REQ_EXPECT_VERSION;
            }
            if (*cursor == '#') { /* empty query */
                ++cursor; /* skips '#' */
                ++(*offset);
                *state = HTTP_REQ_EXPECT_FRAGMENT;
                goto HTTP_REQ_EXPECT_FRAGMENT;
            }

            key.off = cursor - base;
            while (1) {
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
                if (*cursor == '\r' || *cursor == '\n') {
                    return HRC_HEADER;
                }
                if (*cursor == '=') {
                    break;
                }
            }
            key.len = cursor - base - key.off;

            ++cursor;
            if (cursor == end) {
                return HRC_MORE_DATA;
            }

            value.off = cursor - base;
            while (1) {
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
                if (*cursor == '\r' || *cursor == '\n') {
                    return HRC_HEADER;
                }
                if (*cursor == ' ') {
                    value.len = cursor - base - value.off;
                    rc = http_kv_list_update(&l->query_list, base, key.off, key.len,
                                             value.off, value.len);
                    if (rc != HRC_OK) {
                        return rc;
                    }
                    ++cursor; /* skips ' ' */
                    (*offset) += (cursor - last_pos);
                    *state = HTTP_REQ_EXPECT_VERSION;
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_VERSION;
                }
                if (*cursor == '&') {
                    value.len = cursor - base - value.off;
                    rc = http_kv_list_update(&l->query_list, base, key.off, key.len,
                                             value.off, value.len);
                    if (rc != HRC_OK) {
                        return rc;
                    }
                    ++cursor; /* skips '&' */
                    (*offset) += (cursor - last_pos);
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_QUERY;
                }
                if (*cursor == '#') {
                    value.len = cursor - base - value.off;
                    rc = http_kv_list_update(&l->query_list, base, key.off, key.len,
                                             value.off, value.len);
                    if (rc != HRC_OK) {
                        return rc;
                    }
                    *state = HTTP_REQ_EXPECT_FRAGMENT;
                    ++cursor; /* skips '#' */
                    (*offset) += (cursor - last_pos);
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_FRAGMENT;
                }
            }
        }
        HTTP_REQ_EXPECT_FRAGMENT:
        case HTTP_REQ_EXPECT_FRAGMENT: {
            const char* last_pos = cursor;
            while (1) {
                if (*cursor == ' ') {
                    l->fragment.off = last_pos - base;
                    l->fragment.len = cursor - last_pos;
                    ++cursor;
                    (*offset) += (cursor - last_pos);
                    *state = HTTP_REQ_EXPECT_VERSION;
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    goto HTTP_REQ_EXPECT_VERSION;
                }
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
                if (*cursor == '\r' || *cursor == '\n') {
                    return HRC_HEADER;
                }
            }
        }
        HTTP_REQ_EXPECT_VERSION:
        case HTTP_REQ_EXPECT_VERSION: {
            const char* last_pos = cursor;
            while (1) {
                ++cursor;
                if (cursor == end) {
                    return HRC_MORE_DATA;
                }
                if (*cursor == '\r') {
                    ++cursor;
                    if (cursor == end) {
                        return HRC_MORE_DATA;
                    }
                    if (*cursor == '\n') {
                        l->version.off = last_pos - base;
                        l->version.len = cursor - 1 - base - l->version.off;
                        (*offset) += (cursor + 1 - last_pos);
                        return HRC_OK;
                    }
                    return HRC_HEADER;
                }
            }
        }
    }

    return HRC_HEADER;
}

/* ------------------------------------------------------------------------- */

void http_request_decode_context_init(struct http_request_decode_context* ctx) {
    ctx->state = HTTP_REQ_EXPECT_METHOD;
    ctx->base = NULL;
    ctx->offset = 0;
    __request_line_init(&ctx->req_line);
    http_kv_list_init(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
}

void http_request_decode_context_destroy(struct http_request_decode_context* ctx) {
    ctx->state = HTTP_REQ_EXPECT_METHOD;
    ctx->base = NULL;
    ctx->offset = 0;
    __reqeust_line_destroy(&ctx->req_line);
    http_kv_list_destroy(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
}

int http_request_decode(struct http_request_decode_context* ctx, const char* data,
                        unsigned long len) {
    ctx->base = data; /* updates base addr */
    if (ctx->state == HTTP_REQ_EXPECT_END) {
        return HRC_OK;
    }

    data += ctx->offset;
    len -= ctx->offset;

    switch (ctx->state) {
        case HTTP_REQ_EXPECT_METHOD: {
            unsigned long parsed_len = 0;
            int rc = __request_line_decode_method(data, len, &ctx->req_line.method, &parsed_len);
            len -= parsed_len;
            data += parsed_len;
            ctx->offset += parsed_len;
            if (rc != HRC_OK) {
                return rc;
            }
            ctx->state = HTTP_REQ_EXPECT_ABS_PATH;
        }
        case HTTP_REQ_EXPECT_ABS_PATH:
        case HTTP_REQ_EXPECT_QUERY:
        case HTTP_REQ_EXPECT_FRAGMENT:
        case HTTP_REQ_EXPECT_VERSION: {
            unsigned long parsed_len = 0;
            int rc = __request_line_decode_others(ctx->base, data, len, &ctx->req_line,
                                                  &ctx->state, &parsed_len);
            len -= parsed_len;
            data += parsed_len;
            ctx->offset += parsed_len;
            if (rc != HRC_OK) {
                return rc;
            }
            ctx->state = HTTP_REQ_EXPECT_HEADER;
        }
        case HTTP_REQ_EXPECT_HEADER: {
            unsigned long parsed_len = 0;
            int rc = http_header_decode(data, len, ctx->base, &ctx->header_list, &parsed_len);
            len -= parsed_len;
            ctx->offset += parsed_len;
            if (rc != HRC_OK) {
                return rc;
            }

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

void http_request_get_query(const struct http_request_decode_context* ctx, const char* key,
                            unsigned int klen, struct qbuf_ref* value) {
    struct http_item* v = http_kv_list_get(&ctx->req_line.query_list, ctx->base, key, klen);
    if (v) {
        value->base = ctx->base + v->off;
        value->size = v->len;
    } else {
        value->base = NULL;
        value->size = 0;
    }
}

void http_request_get_header(const struct http_request_decode_context* ctx, const char* key,
                             unsigned int klen, struct qbuf_ref* value) {
    struct http_item* v = http_kv_list_get(&ctx->header_list, ctx->base, key, klen);
    if (v) {
        value->base = ctx->base + v->off;
        value->size = v->len;
    } else {
        value->base = NULL;
        value->size = 0;
    }
}
