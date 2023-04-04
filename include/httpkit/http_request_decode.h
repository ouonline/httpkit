#ifndef __HTTPKIT_HTTP_REQUEST_DECODE_H__
#define __HTTPKIT_HTTP_REQUEST_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/list.h"
#include "cutils/qbuf_ref.h"
#include "http_kv_ol_list.h"

struct http_request_line {
    struct qbuf_ol method;
    struct qbuf_ol abs_path;
    struct qbuf_ol fragment;
    struct qbuf_ol version;
    struct http_kv_ol_list query_list;
};

/* values of http_request_decode_context::state */
enum {
    HTTP_REQ_EXPECT_METHOD,
    HTTP_REQ_EXPECT_ABS_PATH,
    HTTP_REQ_EXPECT_QUERY,
    HTTP_REQ_EXPECT_FRAGMENT,
    HTTP_REQ_EXPECT_VERSION,
    HTTP_REQ_EXPECT_HEADER,
    HTTP_REQ_EXPECT_CONTENT,
    HTTP_REQ_EXPECT_END,
};

struct http_request_decode_context {
    int state;

    const char* base;
    unsigned long offset; /* last valid pos */

    struct http_request_line req_line;
    struct http_kv_ol_list header_list;

    unsigned long content_offset;
    unsigned long content_length; /* from header `Content-Length` */
};

/* ------------------------------------------------------------------------- */

void http_request_decode_context_init(struct http_request_decode_context*);
void http_request_decode_context_destroy(struct http_request_decode_context*);
int http_request_decode(struct http_request_decode_context*, const char* data,
                        unsigned long len);

/* update buffers ptr to `base`. Note that `http_request_decode()` will save the `data` addr. */
static inline void http_request_set_buffer_ptr(struct http_request_decode_context* ctx,
                                               const char* data) {
    ctx->base = data;
}

static inline void http_request_get_method(const struct http_request_decode_context* ctx,
                                           struct qbuf_ref* res) {
    res->base = ctx->base + ctx->req_line.method.off;
    res->size = ctx->req_line.method.len;
}

static inline void http_request_get_abs_path(const struct http_request_decode_context* ctx,
                                             struct qbuf_ref* res) {
    res->base = ctx->base + ctx->req_line.abs_path.off;
    res->size = ctx->req_line.abs_path.len;
}

static inline void http_request_get_fragment(const struct http_request_decode_context* ctx,
                                             struct qbuf_ref* res) {
    res->base = ctx->base + ctx->req_line.fragment.off;
    res->size = ctx->req_line.fragment.len;
}

static inline void http_request_get_version(const struct http_request_decode_context* ctx,
                                            struct qbuf_ref* res) {
    res->base = ctx->base + ctx->req_line.version.off;
    res->size = ctx->req_line.version.len;
}

void http_request_get_query(const struct http_request_decode_context*,
                            const char* key, unsigned int klen, struct qbuf_ref* value);

static inline int http_request_for_each_query(const struct http_request_decode_context* ctx, void* arg,
                                              int (*f)(void* arg, const char* key, unsigned int klen,
                                                       const char* value, unsigned int vlen)) {
    return http_kv_ol_list_for_each(&ctx->req_line.query_list, ctx->base, arg, f);
}

void http_request_get_header(const struct http_request_decode_context*,
                             const char* key, unsigned int klen, struct qbuf_ref* value);

static inline int http_request_for_each_header(const struct http_request_decode_context* ctx, void* arg,
                                               int (*f)(void* arg, const char* key, unsigned int klen,
                                                        const char* value, unsigned int vlen)) {
    return http_kv_ol_list_for_each(&ctx->header_list, ctx->base, arg, f);
}

static inline void http_request_get_content(const struct http_request_decode_context* ctx,
                                            struct qbuf_ref* res) {
    res->base = ctx->base + ctx->content_offset;
    res->size = ctx->content_length;
}

static inline unsigned long http_request_get_size(const struct http_request_decode_context* ctx) {
    return ctx->offset;
}

#ifdef __cplusplus
}
#endif

#endif
