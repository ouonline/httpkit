#include "httpkit/http_common.h"
#include "http_header_encode.h"
#include <stdio.h> /* sprintf() */

#define CONTENT_LENGTH_STR "Content-Length"
#define CONTENT_LENGTH_LEN 14

int http_header_encode(const struct http_kv_list* header_list, unsigned long content_len,
                       struct qbuf* res) {
    int no_content_len = 1;

    if (header_list) {
        struct list_node* cur;
        list_for_each(cur, &header_list->head) {
            struct http_kv_node* h = list_entry(cur, struct http_kv_node, node);
            qbuf_append(res, qbuf_data(&h->key), qbuf_size(&h->key));
            qbuf_append(res, ": ", 2);
            qbuf_append(res, qbuf_data(&h->value), qbuf_size(&h->value));
            qbuf_append(res, "\r\n", 2);

            if (no_content_len) {
                if (qbuf_size(&h->key) == CONTENT_LENGTH_LEN &&
                    memcmp(qbuf_data(&h->key), CONTENT_LENGTH_STR, CONTENT_LENGTH_LEN) == 0) {
                    no_content_len = 0;
                }
            }
        }
    }

    if (no_content_len) {
        char tmp[48];
        int tmplen = sprintf(tmp, CONTENT_LENGTH_STR ": %lu\r\n", content_len);
        qbuf_append(res, tmp, tmplen);
    }

    qbuf_append(res, "\r\n", 2); /* header end mark */

    return HRC_OK;

}
