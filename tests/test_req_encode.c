#undef NDEBUG
#include <assert.h>

#include "httpkit/http_common.h"
#include "httpkit/http_request_encode.h"
#include "test_utils.h"

static void test_req_encode1() {
    struct qbuf_ref method = {.base = "GET", .size = 3};
    struct qbuf_ref abs_path = {.base = "/about", .size = 6};

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_request_encode(&method, &abs_path, NULL, NULL, "Hello", 5, &res);
    assert(rc == HRC_OK);

    const char* expected = "GET /about HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

static void test_req_encode_with_option() {
    struct qbuf_ref method = {.base = "POST", .size = 4};
    struct qbuf_ref abs_path = {.base = "/notice", .size = 7};

    struct http_kv_list option_list;
    http_kv_list_init(&option_list);
    make_option2(&option_list);

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_request_encode(&method, &abs_path, &option_list, NULL, "Hello", 5, &res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice?ou=online&foo=bar HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    http_kv_list_destroy(&option_list);
}

static void test_req_encode_with_header() {
    struct qbuf_ref method = {.base = "POST", .size = 4};
    struct qbuf_ref abs_path = {.base = "/notice", .size = 7};

    struct http_kv_list header_list;
    http_kv_list_init(&header_list);
    make_header1_without_content_length(&header_list);

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_request_encode(&method, &abs_path, NULL, &header_list, "Hello", 5, &res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice HTTP/1.1\r\nou: online\r\nContent-Length: 5\r\n\r\nHello";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    http_kv_list_destroy(&header_list);
}

static void test_req_encode_with_option_and_header() {
    struct qbuf_ref method = {.base = "POST", .size = 4};
    struct qbuf_ref abs_path = {.base = "/notice", .size = 7};

    struct http_kv_list option_list;
    http_kv_list_init(&option_list);
    make_option2(&option_list);
    struct http_kv_list header_list;
    http_kv_list_init(&header_list);
    make_header1_without_content_length(&header_list);

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_request_encode(&method, &abs_path, &option_list, &header_list, "Hello", 5, &res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice?ou=online&foo=bar HTTP/1.1\r\nou: online\r\nContent-Length: 5\r\n\r\nHello";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    http_kv_list_destroy(&header_list);
    http_kv_list_destroy(&option_list);
}

static void test_req_encode_with_abs_path_and_option() {
    struct qbuf_ref method = {.base = "POST", .size = 4};
    struct qbuf_ref abs_path = {.base = "/notice?ou=online&foo=bar", .size = 25};

    struct http_kv_list header_list;
    http_kv_list_init(&header_list);
    make_header1_without_content_length(&header_list);

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_request_encode(&method, &abs_path, NULL, &header_list, "Hello", 5, &res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice?ou=online&foo=bar HTTP/1.1\r\nou: online\r\nContent-Length: 5\r\n\r\nHello";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    http_kv_list_destroy(&header_list);
}

void test_req_encode() {
    test_req_encode1();
    test_req_encode_with_option();
    test_req_encode_with_header();
    test_req_encode_with_option_and_header();
    test_req_encode_with_abs_path_and_option();
}
