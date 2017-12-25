#ifndef __HTTP_HEADER_H__
#define __HTTP_HEADER_H__

struct http_header {
    unsigned int content_len;
};

int http_header_init(struct http_header*);
void http_header_destroy(struct http_header*);

void http_header_decode(struct http_header*,
                        const char* data, unsigned int len);

#endif
