#include "httpkit/http_retcode.h"
#include "httpkit/http_response_decode.h"
#include "http_header_decode.h"
#include "misc.h"

/* values of http_response_decode_context::state */
enum {
    HTTP_RES_EXPECT_STATUS_LINE,
    HTTP_RES_EXPECT_HEADER,
    HTTP_RES_EXPECT_CONTENT,
    HTTP_RES_EXPECT_END,
};

static void __http_response_status_line_reset(struct http_response_status_line* st) {
    st->code = 0;
    http_item_reset(&st->text);
    http_item_reset(&st->version);
}

int http_response_decode_context_init(struct http_response_decode_context* ctx) {
    ctx->state = HTTP_RES_EXPECT_STATUS_LINE;
    ctx->offset = 0;
    __http_response_status_line_reset(&ctx->status_line);
    http_kv_list_init(&ctx->header_list);
    ctx->content_offset = 0;
    ctx->content_length = 0;
    return 0;
}

void http_response_decode_context_destroy(struct http_response_decode_context* ctx) {
    ctx->state = HTTP_RES_EXPECT_STATUS_LINE;
    ctx->offset = 0;
    __http_response_status_line_reset(&ctx->status_line);
    http_kv_list_destroy(&ctx->header_list);
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
            return HRC_RES_LINE;
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

    return HRC_RES_LINE; /* unreachable */
}

int http_response_decode(struct http_response_decode_context* ctx, const void* base,
                         unsigned long len) {
    const char* data = (const char*)base;

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
            int rc = http_header_decode(data, len, base, &ctx->header_list, &parsed_len);
            len -= parsed_len;
            ctx->offset += parsed_len;
            if (rc != HRC_OK) {
                return rc;
            }

            set_content_len(base, &ctx->header_list, &ctx->content_length);
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

void http_response_get_header(const struct http_response_decode_context* ctx, const void* base,
                              const char* key, unsigned int klen, struct qbuf_ref* value) {
    struct http_item* v = http_kv_list_get(&ctx->header_list, base, key, klen);
    if (v) {
        value->base = (const char*)base + v->off;
        value->size = v->len;
    } else {
        value->base = NULL;
        value->size = 0;
    }
}
