#ifndef __HTTPKIT_HTTP_RESPONSE_DECODE_H__
#define __HTTPKIT_HTTP_RESPONSE_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/cvector.h"
#include "cutils/offlen.h"

struct http_response_status_line {
    unsigned int code;
    struct offlen text;
    struct offlen version;
};

struct http_response_decode_context {
    /* public */
    unsigned long content_offset;
    unsigned long content_length; /* from header `Content-Length` */
    struct http_response_status_line status_line;
    struct cvector header_list; /* element type of cvector is struct kvpair */
    /* private */
    int state;
    unsigned long offset; /* last valid pos */
};

#ifdef __cplusplus
typedef struct http_response_status_line HttpResponseStatusLine;
typedef struct http_response_decode_context HttpResponseDecodeContext;
#endif

/* ------------------------------------------------------------------------- */

void http_response_decode_context_init(struct http_response_decode_context*);
void http_response_decode_context_destroy(struct http_response_decode_context*);
void http_response_decode_context_clear(struct http_response_decode_context*);

int http_response_decode(struct http_response_decode_context*, const void* base,
                         unsigned long len);

static inline unsigned int http_response_get_status_code(struct http_response_decode_context* ctx) {
    return ctx->status_line.code;
}

static inline void http_response_get_status_text(struct http_response_decode_context* ctx,
                                                 struct offlen* res) {
    *res = ctx->status_line.text;
}

static inline void http_response_get_version(struct http_response_decode_context* ctx,
                                             struct offlen* res) {
    *res = ctx->status_line.version;
}

static inline unsigned int http_response_get_header_count(struct http_response_decode_context* ctx) {
    return cvector_size(&ctx->header_list);
}

void http_response_get_header(struct http_response_decode_context*, unsigned int idx,
                              struct offlen* key /* can be NULL */,
                              struct offlen* value /* can be NULL */);
void http_response_find_header(struct http_response_decode_context*, const void* base,
                               const char* key, unsigned int klen, struct offlen* value);

static inline void http_response_get_content(struct http_response_decode_context* ctx,
                                             struct offlen* res) {
    res->off = ctx->content_offset;
    res->len = ctx->content_length;
}

static inline unsigned long http_response_get_size(struct http_response_decode_context* ctx) {
    return ctx->offset;
}

#ifdef __cplusplus
}
#endif

#endif
