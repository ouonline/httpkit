#ifndef __HTTPKIT_HTTP_RESPONSE_DECODE_H__
#define __HTTPKIT_HTTP_RESPONSE_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http_kv_ol_list.h"
#include "cutils/qbuf_ref.h"

struct http_response_status_line {
    unsigned int code;
    struct qbuf_ol text;
    struct qbuf_ol version;
};

/* values of http_response_decode_context::state */
enum {
    HTTP_RES_EXPECT_STATUS_LINE,
    HTTP_RES_EXPECT_HEADER,
    HTTP_RES_EXPECT_CONTENT,
    HTTP_RES_EXPECT_END,
};

struct http_response_decode_context {
    int state;

    const char* base;
    unsigned long offset; /* last valid pos */

    struct http_response_status_line status_line;
    struct http_kv_ol_list header_list;

    unsigned long content_offset;
    unsigned long content_length; /* from header `Content-Length` */
};

#ifdef __cplusplus
typedef struct http_response_status_line HttpResponseStatusLine;
typedef struct http_response_decode_context HttpResponseDecodeContext;
#endif

/* ------------------------------------------------------------------------- */

int http_response_decode_context_init(struct http_response_decode_context*);
void http_response_decode_context_destroy(struct http_response_decode_context*);
int http_response_decode(struct http_response_decode_context*, const char* data,
                         unsigned long len);

/* update buffers ptr to `base`. Note that `http_response_decode()` will save the `data` addr. */
static inline void http_response_set_buffer_addr(struct http_response_decode_context* ctx,
                                                 const char* data) {
    ctx->base = data;
}

static inline unsigned int http_response_get_status_code(const struct http_response_decode_context* ctx) {
    return ctx->status_line.code;
}

static inline void http_response_get_status_text(const struct http_response_decode_context* ctx,
                                                 struct qbuf_ref* res) {
    res->base = ctx->base + ctx->status_line.text.off;
    res->size = ctx->status_line.text.len;
}

static inline void http_response_get_version(const struct http_response_decode_context* ctx,
                                             struct qbuf_ref* res) {
    res->base = ctx->base + ctx->status_line.version.off;
    res->size = ctx->status_line.version.len;
}

void http_response_get_header(const struct http_response_decode_context*,
                              const char* key, unsigned int klen, struct qbuf_ref* value);

static inline int http_response_for_each_header(const struct http_response_decode_context* ctx, void* arg,
                                                int (*f)(void* arg, const char* key, unsigned int klen,
                                                         const char* value, unsigned int vlen)) {
    return http_kv_ol_list_for_each(&ctx->header_list, ctx->base, arg, f);
}

static inline void http_response_get_content(const struct http_response_decode_context* ctx,
                                             struct qbuf_ref* res) {
    res->base = ctx->base + ctx->content_offset;
    res->size = ctx->content_length;
}

static inline unsigned long http_response_get_size(const struct http_response_decode_context* ctx) {
    return ctx->offset;
}

#ifdef __cplusplus
}
#endif

#endif
