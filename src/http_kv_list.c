#include "httpkit/http_kv_list.h"
#include "httpkit/http_common.h"
#include <stdlib.h> /* malloc/free */

void http_kv_list_init(struct http_kv_list* l) {
    list_init(&l->head);
}

void http_kv_list_destroy(struct http_kv_list* l) {
    struct list_node *cur, *next;
    list_for_each_safe(cur, next, &l->head) {
        struct http_kv_node* kv = list_entry(cur, struct http_kv_node, node);
        __list_del(cur);
        qbuf_destroy(&kv->key);
        qbuf_destroy(&kv->value);
        free(kv);
    }
}

struct qbuf* http_kv_list_get(struct http_kv_list* l, const char* key, unsigned int klen) {
    struct list_node* cur;
    list_for_each(cur, &l->head) {
        struct http_kv_node* node = list_entry(cur, struct http_kv_node, node);
        if (qbuf_size(&node->key) == klen && memcmp(qbuf_data(&node->key), key, klen) == 0) {
            return &node->value;
        }
    }
    return NULL;
}

int http_kv_list_update(struct http_kv_list* l, const char* key, unsigned int klen,
                        const char* value, unsigned int vlen) {
    struct qbuf* res = http_kv_list_get(l, key, klen);
    if (res) {
        qbuf_assign(res, value, vlen);
        return HRC_OK;
    }

    struct http_kv_node* node = malloc(sizeof(struct http_kv_node));
    if (!node) {
        return HRC_NOMEM;
    }

    qbuf_init(&node->key);
    qbuf_assign(&node->key, key, klen);
    qbuf_init(&node->value);
    qbuf_assign(&node->value, value, vlen);
    list_add_prev(&node->node, &l->head);

    return HRC_OK;
}
