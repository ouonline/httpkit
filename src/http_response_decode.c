#include "httpkit/http_common.h"
#include "httpkit/http_response_decode.h"
#include "http_header_decode.h"
#include "misc.h"

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
static int __status_line_decode(const char* base, unsigned long len,
                                struct http_response_status_line* l,
                                unsigned long* offset) {
    const char* cursor = base;
    const char* end = base + len;

    l->version.off = 0;
    while (*cursor != ' ') {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
    }
    l->version.len = cursor - base;

    ++cursor; /* skips ' ' */
    l->code = 0;
    while (1) {
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor == ' ') {
            break;
        }
        if (*cursor < '0' || *cursor > '9') {
            return HRC_RESLINE;
        }
        l->code = l->code * 10 + (*cursor - '0');
        ++cursor;
    }

    l->text.off = cursor + 1 - base;
    while (1) {
        ++cursor;
        if (cursor == end) {
            return HRC_MORE_DATA;
        }
        if (*cursor == '\r') {
            l->text.len = cursor - base - l->text.off;
            ++cursor;
            if (cursor == end) {
                return HRC_MORE_DATA;
            }
            if (*cursor == '\n') {
                (*offset) += (cursor + 1 - base);
                return HRC_OK;
            }
        }
    }

    return HRC_RESLINE; /* unreachable */
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
            unsigned long parsed_len = 0;
            int rc = __status_line_decode(data, len, &ctx->status_line, &parsed_len);
            len -= parsed_len;
            data += parsed_len;
            ctx->offset += parsed_len;
            if (rc != HRC_OK) {
                return rc;
            }
            ctx->state = HTTP_RES_EXPECT_HEADER;
        }
        case HTTP_RES_EXPECT_HEADER: {
            unsigned long parsed_len = 0;
            int rc = http_header_decode(data, len, ctx->base, &ctx->header_list, &parsed_len);
            len -= parsed_len;
            ctx->offset += parsed_len;
            if (rc != HRC_OK) {
                return rc;
            }

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
