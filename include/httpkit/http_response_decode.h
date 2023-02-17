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

struct http_response_decode_context {
#define HTTP_RES_EXPECT_STATUS_LINE 0
#define HTTP_RES_EXPECT_HEADER 1
#define HTTP_RES_EXPECT_CONTENT 2
#define HTTP_RES_EXPECT_END 3
    int state;

    const char* base;
    unsigned long offset;

    struct http_response_status_line status_line;
    struct http_kv_ol_list header_list;

    unsigned long content_offset;
    unsigned long content_length; /* from header `Content-Length` */
};

int http_response_decode_context_init(struct http_response_decode_context*);
void http_response_decode_context_destroy(struct http_response_decode_context*);
int http_response_decode(struct http_response_decode_context*, const char* data,
                         unsigned long len);

unsigned int http_response_get_status_code(const struct http_response_decode_context*);
void http_response_get_status_text(const struct http_response_decode_context*, struct qbuf_ref* res);

void http_response_get_header(const struct http_response_decode_context*,
                              const char* key, unsigned int klen, struct qbuf_ref* value);
int http_response_for_each_header(const struct http_response_decode_context*, void* arg,
                                  int (*f)(void* arg, const char* key, unsigned int klen,
                                           const char* value, unsigned int vlen));

void http_response_get_content(const struct http_response_decode_context*,
                               struct qbuf_ref* res);

unsigned long http_response_get_size(const struct http_response_decode_context*);

#ifdef __cplusplus
}
#endif

#endif
