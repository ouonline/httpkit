#undef NDEBUG
#include <assert.h>

#include "httpkit/http_request_encode.h"

static void test_req_encode1() {
    struct qbuf res;
    qbuf_init(&res);

    int rc = http_request_encode_request_line(&res, "GET", 3, "/about", 6);
    assert(rc == HRC_OK);
    rc = http_request_encode_header(&res, "Content-Length", 14, "5", 1);
    assert(rc == HRC_OK);
    rc = http_request_encode_head_end(&res);
    assert(rc == HRC_OK);

    const char* expected = "GET /about HTTP/1.1\r\nContent-Length: 5\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

static void test_req_encode_with_query() {
    struct qbuf res;
    qbuf_init(&res);

    const char* url = "/notice?ou=online&foo=bar";
    int rc = http_request_encode_request_line(&res, "POST", 4, url, strlen(url));
    assert(rc == HRC_OK);
    rc = http_request_encode_head_end(&res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice?ou=online&foo=bar HTTP/1.1\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

static void test_req_encode_with_header() {
    struct qbuf res;
    qbuf_init(&res);

    int rc = http_request_encode_request_line(&res, "POST", 4, "/notice", 7);
    assert(rc == HRC_OK);
    rc = http_request_encode_header(&res, "ou", 2, "online", 6);
    assert(rc == HRC_OK);
    rc = http_request_encode_header(&res, "Content-Length", 14, "5", 1);
    assert(rc == HRC_OK);
    rc = http_request_encode_head_end(&res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice HTTP/1.1\r\nou: online\r\nContent-Length: 5\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

static void test_req_encode_with_query_and_header() {
    struct qbuf res;
    qbuf_init(&res);

    const char* url = "/notice?ou=online&foo=bar";
    int rc = http_request_encode_request_line(&res, "POST", 4, url, strlen(url));
    assert(rc == HRC_OK);
    rc = http_request_encode_header(&res, "ou", 2, "online", 6);
    assert(rc == HRC_OK);
    rc = http_request_encode_head_end(&res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice?ou=online&foo=bar HTTP/1.1\r\nou: online\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

static void test_req_encode_with_abs_path_and_query() {
    struct qbuf res;
    qbuf_init(&res);

    const char* url = "/notice?ou=online&foo=bar";
    int rc = http_request_encode_request_line(&res, "POST", 4, url, strlen(url));
    assert(rc == HRC_OK);
    rc = http_request_encode_header(&res, "ou", 2, "online", 6);
    assert(rc == HRC_OK);
    rc = http_request_encode_header(&res, "Content-Length", 14, "5", 1);
    assert(rc == HRC_OK);
    rc = http_request_encode_head_end(&res);
    assert(rc == HRC_OK);

    const char* expected = "POST /notice?ou=online&foo=bar HTTP/1.1\r\nou: online\r\nContent-Length: 5\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

void test_req_encode() {
    test_req_encode1();
    test_req_encode_with_query();
    test_req_encode_with_header();
    test_req_encode_with_query_and_header();
    test_req_encode_with_abs_path_and_query();
}
