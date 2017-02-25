#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "list.h"
#include "qbuf.h"

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

struct http_request_header {
    unsigned int content_type;
    unsigned int content_len;
};

struct http_request {
    struct http_request_line req_line;
    struct http_request_header header;
    struct qbuf content;
};

/* request methods */
#define HTTP_REQUEST_METHOD_GET         0
#define HTTP_REQUEST_METHOD_POST        1
#define HTTP_REQUEST_METHOD_UNSUPPORTED 2

/* ------------------------------------------------------------------------- */

int http_request_init(struct http_request*, const char* data,
                      unsigned int len);
void http_request_destroy(struct http_request*);

int http_request_get_method(const struct http_request*);
void http_request_get_abs_path(const struct http_request*, struct qbuf_ref*);
int http_request_for_each_option(const struct http_request*, void* arg,
                                 int (*f)(void* arg,
                                          const char* k, unsigned int klen,
                                          const char* v, unsigned int vlen));

unsigned int http_request_get_content_type(const struct http_request*);
void http_request_get_content(const struct http_request*, struct qbuf_ref*);

/* ----- auxiliary functions ----- */

/* returns an error code or the decoded len in `dst`. */
int http_decode_url(const char* src, unsigned int src_size,
                    char* dst, unsigned int dst_size);

/* returns an error code or the request size. */
int http_get_request_size(const char* data, unsigned int len);

#endif
