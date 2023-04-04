#ifndef __HTTPKIT_HTTP_KV_OL_LIST_H__
#define __HTTPKIT_HTTP_KV_OL_LIST_H__

#include "cutils/list.h"
#include "cutils/qbuf_ol.h"

struct http_kv_ol_node {
    struct qbuf_ol key;
    struct qbuf_ol value;
    struct list_node node;
};

struct http_kv_ol_list {
    struct list_node head;
};

void http_kv_ol_list_init(struct http_kv_ol_list*);
void http_kv_ol_list_destroy(struct http_kv_ol_list*);
int http_kv_ol_list_update(struct http_kv_ol_list*, const char* base,
                           unsigned int koff, unsigned int klen,
                           unsigned int voff, unsigned int vlen);
struct qbuf_ol* http_kv_ol_list_get(const struct http_kv_ol_list*, const char* base,
                                    const char* key, unsigned int klen);
int http_kv_ol_list_for_each(const struct http_kv_ol_list*, const char* base, void* arg,
                             int (*f)(void* arg,
                                      const char* k, unsigned int klen,
                                      const char* v, unsigned int vlen));

#endif
