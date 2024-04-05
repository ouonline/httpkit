#ifndef __HTTPKIT_HTTP_KV_LIST_H__
#define __HTTPKIT_HTTP_KV_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http_item.h"
#include "cutils/list.h"

struct http_kv_node {
    struct http_item key;
    struct http_item value;
    struct list_node node;
};

struct http_kv_list {
    struct list_node head;
};

void http_kv_list_init(struct http_kv_list*);
void http_kv_list_destroy(struct http_kv_list*);
int http_kv_list_update(struct http_kv_list*, const void* base,
                        unsigned int koff, unsigned int klen,
                        unsigned int voff, unsigned int vlen);
struct http_item* http_kv_list_get(const struct http_kv_list*, const void* base,
                                   const char* key, unsigned int klen);
int http_kv_list_for_each(const struct http_kv_list*, const void* base, void* arg,
                          int (*f)(void* arg,
                                   const char* k, unsigned int klen,
                                   const char* v, unsigned int vlen));

#ifdef __cplusplus
}
#endif

#endif
