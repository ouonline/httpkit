#include "httpkit/http_kv_list.h"
#include "httpkit/http_retcode.h"
#include <string.h> /* memcmp() */
#include <stdlib.h> /* malloc/free */

void http_kv_list_init(struct http_kv_list* l) {
    list_init(&l->head);
}

void http_kv_list_destroy(struct http_kv_list* l) {
    struct list_node *cur, *next;
    list_for_each_safe(cur, next, &l->head) {
        struct http_kv_node* node = list_entry(cur, struct http_kv_node, node);
        __list_del(cur);
        free(node);
    }
}

static struct http_kv_node* __get(const struct http_kv_list* l, const char* base,
                                  const char* key, unsigned int klen) {
    struct list_node* cur;
    list_for_each(cur, &l->head) {
        struct http_kv_node* node = list_entry(cur, struct http_kv_node, node);
        if (klen == node->key.len) {
            const char* kstr = base + node->key.off;
            if (memcmp(kstr, key, klen) == 0) {
                return node;
            }
        }
    }
    return NULL;
}

struct http_item* http_kv_list_get(const struct http_kv_list* l, const char* base,
                                   const char* key, unsigned int klen) {
    struct http_kv_node* node = __get(l, base, key, klen);
    if (node) {
        return &node->value;
    }
    return NULL;
}

int http_kv_list_update(struct http_kv_list* l, const char* base,
                        unsigned int koff, unsigned int klen,
                        unsigned int voff, unsigned int vlen) {
    const char* key = base + koff;
    struct http_kv_node* node = __get(l, base, key, klen);
    if (node) {
        node->key.off = koff; /* update to the latest offset */
        node->value.off = voff;
        node->value.len = vlen;
        return HRC_OK;
    }

    node = malloc(sizeof(struct http_kv_node));
    if (!node) {
        return HRC_NOMEM;
    }

    node->key.off = koff;
    node->key.len = klen;
    node->value.off = voff;
    node->value.len = vlen;
    list_add_prev(&node->node, &l->head);

    return HRC_OK;
}

int http_kv_list_for_each(const struct http_kv_list* l, const char* base, void* arg,
                          int (*f)(void* arg,
                                   const char* k, unsigned int klen,
                                   const char* v, unsigned int vlen)) {
    struct list_node* cur;
    list_for_each(cur, &l->head) {
        struct http_kv_node* node = list_entry(cur, struct http_kv_node, node);
        int rc = f(arg, base + node->key.off, node->key.len,
                   base + node->value.off, node->value.len);
        if (rc != HRC_OK) {
            return rc;
        }
    }

    return HRC_OK;
}
