#include "httpkit/http_common.h"
#include "httpkit/http_response_decode.h"
#include "http_header_decode.h"
#include "misc.h"
#include "cutils/str_utils.h" /* ndec2long()/memmem() */

static void __http_response_status_line_reset(struct http_response_status_line* st) {
    st->code = 0;
    qbuf_ol_reset(&st->text);
    qbuf_ol_reset(&st->version);
}

int http_response_decode_context_init(struct http_response_decode_context* ctx) {
    ctx->state = HTTP_RES_EXPECT_STATUS_LINE;
    ctx->base = NULL;
    ctx->offset = 0;
    __http_response_status_line_reset(&ctx->status_line);
    http_kv_ol_list_init(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
    return 0;
}

void http_response_decode_context_destroy(struct http_response_decode_context* ctx) {
    ctx->state = HTTP_RES_EXPECT_STATUS_LINE;
    ctx->base = NULL;
    ctx->offset = 0;
    __http_response_status_line_reset(&ctx->status_line);
    http_kv_ol_list_destroy(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
}

/* Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF */
static int __status_line_decode(const char* data, unsigned int len,
                                struct http_response_status_line* st_line) {
    const char* base = data;
    const char* cursor = memmem(data, len, " ", 1);
    if (!cursor) {
        return HRC_RESLINE;
    }

    st_line->version.off = 0;
    st_line->version.len = cursor - data;

    len -= (cursor - data + 1);
    data = cursor + 1; /* skip space */

    cursor = memmem(data, len, " ", 1);
    if (!cursor) {
        return HRC_RESLINE;
    }
    if (cursor - data != 3) { /* 3 digits */
        return HRC_RESLINE;
    }

    st_line->code = ndec2long(data, 3);

    len -= (cursor - data + 1);
    data = cursor + 1;

    st_line->text.off = data - base;
    st_line->text.len = len;

    return HRC_OK;
}

int http_response_decode(struct http_response_decode_context* ctx, const char* data,
                         unsigned long len) {
    ctx->base = data;
    if (ctx->state == HTTP_RES_EXPECT_END) {
        return HRC_OK;
    }

    data += ctx->offset;
    len -= ctx->offset;

    switch (ctx->state) {
        case HTTP_RES_EXPECT_STATUS_LINE: {
            const char* cursor = memmem(data, len, "\r\n", 2);
            if (!cursor) {
                return HRC_MORE_DATA;
            }
            if (cursor == data) {
                return HRC_RESLINE;
            }

            int rc = __status_line_decode(data, cursor - data, &ctx->status_line);
            if (rc != HRC_OK) {
                return rc;
            }

            len -= (cursor + 2 - data);
            ctx->offset += (cursor + 2 - data);
            data = cursor + 2 /* skip '\r\n' */;
            ctx->state = HTTP_RES_EXPECT_HEADER;
        }
        case HTTP_RES_EXPECT_HEADER: {
            unsigned long offset_before = ctx->offset;
            int rc = http_header_decode(data, len, ctx->base, &ctx->header_list,
                                        &ctx->offset);
            if (rc != HRC_OK) {
                return rc;
            }

            len -= (ctx->offset - offset_before);
            set_content_len(ctx->base, &ctx->header_list, &ctx->content_length);
            ctx->state = HTTP_RES_EXPECT_CONTENT;
        }
        case HTTP_RES_EXPECT_CONTENT: {
            if (len < ctx->content_length) {
                return HRC_MORE_DATA;
            }
            ctx->content_offset = ctx->offset;
            ctx->offset += ctx->content_length;
            ctx->state = HTTP_RES_EXPECT_END;
        }
    }

    return HRC_OK;
}

unsigned int http_response_get_status_code(const struct http_response_decode_context* ctx) {
    return ctx->status_line.code;
}

void http_response_get_status_text(const struct http_response_decode_context* ctx,
                                   struct qbuf_ref* res) {
    res->base = ctx->base + ctx->status_line.text.off;
    res->size = ctx->status_line.text.len;
}

void http_response_get_text(const struct http_response_decode_context* ctx,
                            struct qbuf_ref* res) {
    res->base = ctx->base + ctx->status_line.text.off;
    res->size = ctx->status_line.text.len;
}

void http_response_get_header(const struct http_response_decode_context* ctx, const char* key,
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

int http_response_for_each_header(const struct http_response_decode_context* ctx, void* arg,
                                 int (*f)(void* arg,
                                          const char* k, unsigned int klen,
                                          const char* v, unsigned int vlen)) {
    return http_kv_ol_list_for_each(&ctx->header_list, ctx->base, arg, f);
}

void http_response_get_content(const struct http_response_decode_context* ctx,
                               struct qbuf_ref* ref) {
    if (ctx->state == HTTP_RES_EXPECT_END) {
        ref->base = ctx->base + ctx->content_offset;
        ref->size = ctx->content_length;
    } else {
        ref->base = NULL;
        ref->size = 0;
    }
}

unsigned long http_response_get_size(const struct http_response_decode_context* ctx) {
    if (ctx->state == HTTP_RES_EXPECT_END) {
        return ctx->offset;
    }
    return 0;
}
