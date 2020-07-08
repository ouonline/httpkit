#ifndef __HTTPKIT_HTTP_HEADER_H__
#define __HTTPKIT_HTTP_HEADER_H__

#ifdef __cplusplus
extern "C" {
#endif

struct http_header {
    unsigned int content_len;
};

#ifdef __cplusplus
typedef struct http_header HttpHeader;
#endif

int http_header_init(struct http_header*);
void http_header_destroy(struct http_header*);

void http_header_decode(struct http_header*,
                        const char* data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
