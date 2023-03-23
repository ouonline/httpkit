#include "httpkit/http_request_encode.h"
#include "httpkit/http_common.h"
#include "http_header_encode.h"

static int __append_queries(const struct http_kv_list* query_list, struct qbuf* res) {
    if (!query_list || list_empty(&query_list->head)) {
        return HRC_OK;
    }

    struct list_node* cur = list_first(&query_list->head);
    struct http_kv_node* opt = list_entry(cur, struct http_kv_node, node);
    qbuf_append_c(res, '?');
    qbuf_append(res, qbuf_data(&opt->key), qbuf_size(&opt->key));
    qbuf_append_c(res, '=');
    qbuf_append(res, qbuf_data(&opt->value), qbuf_size(&opt->value));

    list_for_each_from(cur, list_next(cur), &query_list->head) {
        opt = list_entry(cur, struct http_kv_node, node);
        qbuf_append_c(res, '&');
        qbuf_append(res, qbuf_data(&opt->key), qbuf_size(&opt->key));
        qbuf_append_c(res, '=');
        qbuf_append(res, qbuf_data(&opt->value), qbuf_size(&opt->value));
    }

    return HRC_OK;
}

static int __append_req_line(const struct qbuf_ref* method, const struct qbuf_ref* abs_path,
                             const struct http_kv_list* query_list, struct qbuf* res) {
    qbuf_append(res, method->base, method->size);
    qbuf_append_c(res, ' ');

    if (abs_path->size == 0) {
        qbuf_append_c(res, '/');
    } else {
        qbuf_append(res, abs_path->base, abs_path->size);
    }

    int err = __append_queries(query_list, res);
    if (err) {
        return HRC_NOMEM;
    }

    err = qbuf_append(res, " HTTP/1.1\r\n", 11);
    if (err) {
        return HRC_NOMEM;
    }

    return HRC_OK;
}

int http_request_encode_head(const struct qbuf_ref* method, const struct qbuf_ref* abs_path,
                             const struct http_kv_list* query_list,
                             const struct http_kv_list* header_list,
                             unsigned long content_len, struct qbuf* res) {
    int err = qbuf_reserve(res, qbuf_size(res) + 128 + content_len);
    if (err) {
        return HRC_NOMEM;
    }

    int rc = __append_req_line(method, abs_path, query_list, res);
    if (rc != HRC_OK) {
        return rc;
    }

    rc = http_header_encode(header_list, content_len, res);
    if (rc != HRC_OK) {
        return rc;
    }

    return HRC_OK;
}
