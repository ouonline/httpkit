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
    struct http_kv_ol_list option_list;
};

struct http_request_decode_context {
#define HTTP_REQ_EXPECT_REQLINE 0
#define HTTP_REQ_EXPECT_HEADER 1
#define HTTP_REQ_EXPECT_CONTENT 2
#define HTTP_REQ_EXPECT_END 3
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

void http_request_get_method(const struct http_request_decode_context*,
                             struct qbuf_ref* res);
void http_request_get_abs_path(const struct http_request_decode_context*,
                               struct qbuf_ref* res);

void http_request_get_option(const struct http_request_decode_context*,
                             const char* key, unsigned int klen, struct qbuf_ref* value);
int http_request_for_each_option(const struct http_request_decode_context*, void* arg,
                                 int (*f)(void* arg, const char* key, unsigned int klen,
                                          const char* value, unsigned int vlen));

void http_request_get_header(const struct http_request_decode_context*,
                             const char* key, unsigned int klen, struct qbuf_ref* value);
int http_request_for_each_header(const struct http_request_decode_context*, void* arg,
                                 int (*f)(void* arg, const char* key, unsigned int klen,
                                          const char* value, unsigned int vlen));

void http_request_get_content(const struct http_request_decode_context*, struct qbuf_ref*);

unsigned long http_request_get_size(const struct http_request_decode_context*);

#ifdef __cplusplus
}
#endif

#endif
