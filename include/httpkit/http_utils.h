#ifndef __HTTPKIT_HTTP_UTILS_H__
#define __HTTPKIT_HTTP_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* returns an error code or the decoded len in `dst`. */
int http_decode_url(const char* src, unsigned int src_size,
                    char* dst, unsigned int dst_size);

#ifdef __cplusplus
}
#endif

#endif
