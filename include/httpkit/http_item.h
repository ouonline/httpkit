#ifndef __HTTPKIT_HTTP_ITEM_H__
#define __HTTPKIT_HTTP_ITEM_H__

#ifdef __cplusplus
extern "C" {
#endif

struct http_item {
    unsigned int off;
    unsigned int len;
};

static inline void http_item_reset(struct http_item* item) {
    item->off = 0;
    item->len = 0;
}

#ifdef __cplusplus
}
#endif

#endif
