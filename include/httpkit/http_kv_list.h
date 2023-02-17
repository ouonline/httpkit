#ifndef __HTTPKIT_HTTP_KV_LIST_H__
#define __HTTPKIT_HTTP_KV_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cutils/list.h"
#include "cutils/qbuf.h"

struct http_kv_node {
    struct qbuf key;
    struct qbuf value;
    struct list_node node;
};

struct http_kv_list {
    struct list_node head;
};

void http_kv_list_init(struct http_kv_list*);
void http_kv_list_destroy(struct http_kv_list*);
int http_kv_list_update(struct http_kv_list*, const char* key, unsigned int klen,
                        const char* value, unsigned int vlen);
struct qbuf* http_kv_list_get(struct http_kv_list*, const char* key, unsigned int klen);

#ifdef __cplusplus
}
#endif

#endif
