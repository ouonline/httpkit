#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

/* returns an error code or the decoded len in `dst`. */
int http_decode_url(const char* src, unsigned int src_size,
                    char* dst, unsigned int dst_size);

#endif
