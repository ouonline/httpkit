#undef NDEBUG
#include <assert.h>
#include <string.h>

#include "httpkit/http_common.h"
#include "../src/http_header_encode.h"
#include "test_utils.h"

static void test_header_encode_with_content_length() {
    unsigned long content_len = 5;

    struct http_kv_list header_list;
    http_kv_list_init(&header_list);
    make_header1_with_content_length(&header_list, content_len);

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_header_encode(&header_list, content_len, &res);
    assert(rc == HRC_OK);

    const char* expected = "Content-Length: 5\r\nou: online\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    http_kv_list_destroy(&header_list);
}

static void test_header_encode_without_content_length() {
    struct http_kv_list header_list;
    http_kv_list_init(&header_list);
    make_header1_without_content_length(&header_list);

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_header_encode(&header_list, 0, &res);
    assert(rc == HRC_OK);

    const char* expected = "ou: online\r\nContent-Length: 0\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    http_kv_list_destroy(&header_list);
}

void test_header_encode() {
    test_header_encode_with_content_length();
    test_header_encode_without_content_length();
}
