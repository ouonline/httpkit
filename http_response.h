#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include "utils/qbuf.h"

struct http_response {
    struct qbuf buf;
};

int http_response_init(struct http_response*);
void http_response_destroy(struct http_response*);

void http_response_get_data(struct http_response*, struct qbuf_ref*);

int http_response_pack(struct http_response*, unsigned int status_code,
                       unsigned int content_type,
                       const char* content, unsigned int content_len);

#endif
