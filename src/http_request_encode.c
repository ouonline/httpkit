#include "httpkit/http_request_encode.h"
#include "httpkit/http_common.h"
#include "http_header_encode.h"

static int __append_options(const struct http_kv_list* option_list, struct qbuf* res) {
    if (!option_list || list_empty(&option_list->head)) {
        return HRC_OK;
    }

    struct list_node* cur = list_first(&option_list->head);
    struct http_kv_node* opt = list_entry(cur, struct http_kv_node, node);
    qbuf_append_c(res, '?');
    qbuf_append(res, qbuf_data(&opt->key), qbuf_size(&opt->key));
    qbuf_append_c(res, '=');
    qbuf_append(res, qbuf_data(&opt->value), qbuf_size(&opt->value));

    list_for_each_from(cur, list_next(cur), &option_list->head) {
        opt = list_entry(cur, struct http_kv_node, node);
        qbuf_append_c(res, '&');
        qbuf_append(res, qbuf_data(&opt->key), qbuf_size(&opt->key));
        qbuf_append_c(res, '=');
        qbuf_append(res, qbuf_data(&opt->value), qbuf_size(&opt->value));
    }

    return HRC_OK;
}

static int __append_req_line(const struct qbuf_ref* method, const struct qbuf_ref* abs_path,
                             const struct http_kv_list* option_list, struct qbuf* res) {
    qbuf_append(res, method->base, method->size);
    qbuf_append_c(res, ' ');

    if (abs_path->size == 0) {
        qbuf_append_c(res, '/');
    } else {
        qbuf_append(res, abs_path->base, abs_path->size);
    }

    int err = __append_options(option_list, res);
    if (err) {
        return HRC_NOMEM;
    }

    err = qbuf_append(res, " HTTP/1.1\r\n", 11);
    if (err) {
        return HRC_NOMEM;
    }

    return HRC_OK;
}

int http_request_encode(const struct qbuf_ref* method, const struct qbuf_ref* abs_path,
                        const struct http_kv_list* option_list,
                        const struct http_kv_list* header_list,
                        const char* content, unsigned long content_len, struct qbuf* res) {
    int err = qbuf_reserve(res, qbuf_size(res) + 128 + content_len);
    if (err) {
        return HRC_NOMEM;
    }

    int rc = __append_req_line(method, abs_path, option_list, res);
    if (rc != HRC_OK) {
        return rc;
    }

    rc = http_header_encode(header_list, content_len, res);
    if (rc != HRC_OK) {
        return rc;
    }

    if (content_len > 0) {
        qbuf_append(res, content, content_len);
    }

    return HRC_OK;
}
