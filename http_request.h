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

/* request content types */
#define HTTP_CONTENT_TYPE_PLAIN       0 /* text/plain */
#define HTTP_CONTENT_TYPE_JSON        1 /* application/json */
#define HTTP_CONTENT_TYPE_FORM        2 /* application/x-www-form-urlencoded */
#define HTTP_CONTENT_TYPE_XML         3 /* text/xml */
#define HTTP_CONTENT_TYPE_HTML        4 /* text/html */
#define HTTP_CONTENT_TYPE_UNSUPPORTED 5

/* error code */
#define HRE_SUCCESS      0
#define HRE_NOMEM        (-1)  /* out of memory */
#define HRE_REQLINE      (-2)  /* request line error */
#define HRE_REQMETHOD    (-3)  /* unknown/unsupported request method */
#define HRE_URLDECODE    (-4)  /* url decoding error */
#define HRE_HEADER       (-5)  /* invalid request header */
#define HRE_EMPTY        (-6)  /* empty argument/result */
#define HRE_HTTP_STATUS  (-7)  /* invalid http status */
#define HRE_CONTENT_TYPE (-8)  /* invalid content type */
#define HRE_CONTENT_SIZE (-9)  /* content size mismatch */
#define HRE_BUFSIZE      (-10) /* invalid buffer size */

enum {
    HTTP_STATUS_100 = 0,
    HTTP_STATUS_101,

    HTTP_STATUS_200,
    HTTP_STATUS_201,
    HTTP_STATUS_202,
    HTTP_STATUS_203,
    HTTP_STATUS_204,
    HTTP_STATUS_205,
    HTTP_STATUS_206,

    HTTP_STATUS_300,
    HTTP_STATUS_301,
    HTTP_STATUS_302,
    HTTP_STATUS_303,
    HTTP_STATUS_304,
    HTTP_STATUS_305,
    HTTP_STATUS_307,

    HTTP_STATUS_400,
    HTTP_STATUS_401,
    HTTP_STATUS_402,
    HTTP_STATUS_403,
    HTTP_STATUS_404,
    HTTP_STATUS_405,
    HTTP_STATUS_406,
    HTTP_STATUS_407,
    HTTP_STATUS_408,
    HTTP_STATUS_409,
    HTTP_STATUS_410,
    HTTP_STATUS_411,
    HTTP_STATUS_412,
    HTTP_STATUS_413,
    HTTP_STATUS_414,
    HTTP_STATUS_415,
    HTTP_STATUS_416,
    HTTP_STATUS_417,

    HTTP_STATUS_500,
    HTTP_STATUS_501,
    HTTP_STATUS_502,
    HTTP_STATUS_503,
    HTTP_STATUS_504,
    HTTP_STATUS_505,

    HTTP_STATUS_MAX,
};

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

/* returns an error code or the max size of response. */
int http_get_response_max_size(unsigned int status_code,
                               unsigned int content_type,
                               unsigned int content_len);

/* returns an error code or the response size in `buf`. */
int http_pack_response(unsigned int status_code, unsigned int content_type,
                       const char* content, unsigned int content_len,
                       char* dst, unsigned int dst_len);

#endif
