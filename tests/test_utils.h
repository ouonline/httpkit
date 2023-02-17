#ifndef __TESTS_TEST_UTILS_H__
#define __TESTS_TEST_UTILS_H__

#include "httpkit/http_kv_list.h"
#include <stdio.h> /* sprintf() */

static inline void make_option1(struct http_kv_list* opts) {
    http_kv_list_update(opts, "ou", 2, "online", 6);
}

static inline void make_option2(struct http_kv_list* opts) {
    http_kv_list_update(opts, "ou", 2, "online", 6);
    http_kv_list_update(opts, "foo", 3, "bar", 3);
}

static inline void make_header1_without_content_length(struct http_kv_list* hdrs) {
    http_kv_list_update(hdrs, "ou", 2, "online", 6);
}

static inline void make_header1_with_content_length(struct http_kv_list* hdrs,
                                                    unsigned long content_len) {
    char tmp[32];
    int len = sprintf(tmp, "%lu", content_len);
    http_kv_list_update(hdrs, "Content-Length", 14, tmp, len);

    http_kv_list_update(hdrs, "ou", 2, "online", 6);
}

#endif
