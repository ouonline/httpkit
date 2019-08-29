#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "deps/utils/qbuf.h"
#include "http_header.h"

struct http_response_status {
    unsigned int code;
    struct qbuf_ref text;
    struct qbuf_ref http_version;
};

struct http_response {
    struct http_response_status status_line;
    struct http_header header;
    struct qbuf_ref content;
    struct qbuf raw_data;
};

int http_response_init(struct http_response*);
void http_response_destroy(struct http_response*);

unsigned int http_response_get_status_code(const struct http_response*);
void http_response_get_content(const struct http_response*, struct qbuf_ref*);
void http_response_get_packet(const struct http_response*, struct qbuf_ref*);

int http_response_encode(struct http_response*, unsigned int status_code,
                         const char* content, unsigned int content_len);
int http_response_decode(struct http_response*, const char* data,
                         unsigned int len);

/* ----- auxiliary functions ----- */

int http_get_response_size(const char* data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
