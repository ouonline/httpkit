#ifndef __HTTPKIT_HTTP_REQUEST_DECODE_H__
#define __HTTPKIT_HTTP_REQUEST_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/qbuf_ref.h"
#include "cutils/cvector.h"
#include "http_item.h"

struct http_request_line {
    struct http_item method;
    struct http_item abs_path;
    struct http_item fragment;
    struct http_item version;
    struct cvector query_list; /* element type is struct kvpair */
};

struct http_request_decode_context {
    /* public */
    unsigned long content_offset;
    unsigned long content_length; /* from header `Content-Length` */
    struct http_request_line req_line;
    struct cvector header_list; /* element type is struct kvpair */
    /* private */
    int state;
    unsigned long offset; /* last valid pos */
};

#ifdef __cplusplus
typedef struct http_request_line HttpRequestLine;
typedef struct http_request_decode_context HttpRequestDecodeContext;
#endif

/* ------------------------------------------------------------------------- */

void http_request_decode_context_init(struct http_request_decode_context*);
void http_request_decode_context_destroy(struct http_request_decode_context*);
int http_request_decode(struct http_request_decode_context*, const void* base,
                        unsigned long len);

static inline void http_request_get_method(struct http_request_decode_context* ctx,
                                           const void* base, struct qbuf_ref* res) {
    res->base = (const char*)base + ctx->req_line.method.off;
    res->size = ctx->req_line.method.len;
}

static inline void http_request_get_abs_path(struct http_request_decode_context* ctx,
                                             const void* base, struct qbuf_ref* res) {
    res->base = (const char*)base + ctx->req_line.abs_path.off;
    res->size = ctx->req_line.abs_path.len;
}

static inline void http_request_get_fragment(struct http_request_decode_context* ctx,
                                             const void* base, struct qbuf_ref* res) {
    res->base = (const char*)base + ctx->req_line.fragment.off;
    res->size = ctx->req_line.fragment.len;
}

static inline void http_request_get_version(struct http_request_decode_context* ctx,
                                            const void* base, struct qbuf_ref* res) {
    res->base = (const char*)base + ctx->req_line.version.off;
    res->size = ctx->req_line.version.len;
}

void http_request_get_query(struct http_request_decode_context*, const void* base,
                            const char* key, unsigned int klen, struct qbuf_ref* value);

int http_request_for_each_query(struct http_request_decode_context* ctx, const void* base,
                                void* arg, int (*f)(void* arg, const char* key, unsigned int klen,
                                                    const char* value, unsigned int vlen));

void http_request_get_header(struct http_request_decode_context*, const void* base,
                             const char* key, unsigned int klen, struct qbuf_ref* value);

int http_request_for_each_header(struct http_request_decode_context* ctx, const void* base,
                                 void* arg, int (*f)(void* arg, const char* key, unsigned int klen,
                                                     const char* value, unsigned int vlen));

static inline void http_request_get_content(struct http_request_decode_context* ctx,
                                            const void* base, struct qbuf_ref* res) {
    res->base = (const char*)base + ctx->content_offset;
    res->size = ctx->content_length;
}

static inline unsigned long http_request_get_size(struct http_request_decode_context* ctx) {
    return ctx->offset;
}

#ifdef __cplusplus
}
#endif

#endif
