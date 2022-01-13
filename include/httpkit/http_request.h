#ifndef __HTTPKIT_HTTP_REQUEST_H__
#define __HTTPKIT_HTTP_REQUEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/list.h"
#include "cutils/qbuf.h"
#include "http_header.h"

struct http_request_option {
    struct qbuf key;
    struct qbuf value;
    struct list_node node;
};

struct http_request_line {
    unsigned int method;
    struct qbuf abs_path;
    struct list_node option_list;
};

struct http_request {
    struct http_request_line req_line;
    struct http_header header;
    struct qbuf_ref content;
    struct qbuf raw_data;
};

#ifdef __cplusplus
typedef struct http_request_option HttpRequestOption;
typedef struct http_request_line HttpRequestLine;
typedef struct http_request HttpRequest;
#endif

/* request methods */
#define HTTP_REQUEST_METHOD_GET         0
#define HTTP_REQUEST_METHOD_POST        1
#define HTTP_REQUEST_METHOD_UNSUPPORTED 2

/* ------------------------------------------------------------------------- */

int http_request_init(struct http_request*);
void http_request_destroy(struct http_request*);

unsigned int http_request_get_method(const struct http_request*);
void http_request_set_method(struct http_request*, unsigned int);

void http_request_get_abs_path(const struct http_request*, struct qbuf_ref*);
int http_request_set_abs_path(struct http_request*, const char* data,
                              unsigned int len);

int http_request_for_each_option(const struct http_request*, void* arg,
                                 int (*f)(void* arg,
                                          const char* k, unsigned int klen,
                                          const char* v, unsigned int vlen));
int http_request_append_option(struct http_request*,
                               const char* key, unsigned int klen,
                               const char* value, unsigned int vlen);

int http_request_decode(struct http_request*, const char* data, unsigned int len);
int http_request_encode(struct http_request*,
                        const char* content, unsigned int content_len);

void http_request_get_content(const struct http_request*, struct qbuf_ref*);
void http_request_get_packet(const struct http_request*, struct qbuf_ref*);

/* ----- auxiliary functions ----- */

/* returns an error code or the request size. */
int http_get_request_size(const char* data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
