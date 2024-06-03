#ifndef __HTTPKIT_HTTP_REQUEST_DECODE_H__
#define __HTTPKIT_HTTP_REQUEST_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/cvector.h"
#include "cutils/offlen.h"

struct http_request_line {
    struct offlen method;
    struct offlen abs_path;
    struct offlen fragment;
    struct offlen version;
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
                                           struct offlen* res) {
    *res = ctx->req_line.method;
}

static inline void http_request_get_abs_path(struct http_request_decode_context* ctx,
                                             struct offlen* res) {
    *res = ctx->req_line.abs_path;
}

static inline void http_request_get_fragment(struct http_request_decode_context* ctx,
                                             struct offlen* res) {
    *res = ctx->req_line.fragment;
}

static inline void http_request_get_version(struct http_request_decode_context* ctx,
                                            struct offlen* res) {
    *res = ctx->req_line.version;
}

static inline unsigned int http_request_get_query_count(struct http_request_decode_context* ctx) {
    return cvector_size(&ctx->req_line.query_list);
}

void http_request_get_query(struct http_request_decode_context*, unsigned int idx,
                            struct offlen* key /* can be NULL */,
                            struct offlen* value /* can be NULL */);
void http_request_find_query(struct http_request_decode_context*, const void* base,
                             const char* key, unsigned int klen, struct offlen* value);

static inline unsigned int http_request_get_header_count(struct http_request_decode_context* ctx) {
    return cvector_size(&ctx->header_list);
}

void http_request_get_header(struct http_request_decode_context*, unsigned int idx,
                             struct offlen* key /* can be NULL */,
                             struct offlen* value /* can be NULL */);
void http_request_find_header(struct http_request_decode_context*, const void* base,
                              const char* key, unsigned int klen, struct offlen* value);

static inline void http_request_get_content(struct http_request_decode_context* ctx,
                                            struct offlen* res) {
    res->off = ctx->content_offset;
    res->len = ctx->content_length;
}

static inline unsigned long http_request_get_size(struct http_request_decode_context* ctx) {
    return ctx->offset;
}

#ifdef __cplusplus
}
#endif

#endif
