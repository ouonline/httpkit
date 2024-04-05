#undef NDEBUG
#include <assert.h>

#include "httpkit/http_retcode.h"
#include "httpkit/http_request_decode.h"
#include <string.h>

void test_req_decode1() {
    const char* req = "GET /about HTTP/1.1\r\nContent-Length: 8\r\n\r\nouonline";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_method(&ctx, req, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "GET", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_abs_path(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "/about", 6) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "Content-Length", 14, &ref);
    assert(ref.size == 1);
    assert(memcmp(ref.base, "8", 1) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_content(&ctx, req, &ref);
    assert(ref.size == 8);
    assert(memcmp(ref.base, "ouonline", 8) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_version(&ctx, req, &ref);
    assert(ref.size == 8);
    assert(memcmp(ref.base, "HTTP/1.1", 8) == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_query() {
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nContent-Length: 0\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_query(&ctx, req, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_query(&ctx, req, "bar", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "baz", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_query(&ctx, req, "unknown", 7, &ref);
    assert(ref.base == NULL);
    assert(ref.size == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_header() {
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "abc", 3, &ref);
    assert(ref.size == 4);
    assert(memcmp(ref.base, "defg", 4) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_more_data() {
    const char* req1 = "GET /about?foo=bar&ba";
    const char* req2 = "GET /about?foo=bar&bar=baz HTTP/1.1\r";
    const char* req3 = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: ";
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\nContent-Length: 5\r\n\r\nhello";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req1, strlen(req1));
    assert(rc == HRC_MORE_DATA);

    rc = http_request_decode(&ctx, req2, strlen(req2));
    assert(rc == HRC_MORE_DATA);

    rc = http_request_decode(&ctx, req3, strlen(req3));
    assert(rc == HRC_MORE_DATA);

    rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "abc", 3, &ref);
    assert(ref.size == 4);
    assert(memcmp(ref.base, "defg", 4) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_query(&ctx, req, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_query(&ctx, req, "bar", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "baz", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_content(&ctx, req, &ref);
    assert(ref.size == 5);
    assert(memcmp(ref.base, "hello", 5) == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_no_content_len() {
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_invalid_content_len() {
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nContent-Length: 111\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_MORE_DATA);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_fragment() {
    const char* req = "GET /about?foo=bar&bar=baz#anchor HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_query(&ctx, req, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_fragment(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "anchor", 6) == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_fragment2() {
    const char* req = "GET /about#anchor HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_abs_path(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "/about", 6) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "abc", 3, &ref);
    assert(ref.size == 4);
    assert(memcmp(ref.base, "defg", 4) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_fragment(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "anchor", 6) == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_empty_fragment() {
    const char* req = "GET /about# HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_abs_path(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "/about", 6) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "abc", 3, &ref);
    assert(ref.size == 4);
    assert(memcmp(ref.base, "defg", 4) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_fragment(&ctx, req, &ref);
    assert(ref.size == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_empty_query() {
    const char* req = "GET /about? HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_abs_path(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "/about", 6) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "abc", 3, &ref);
    assert(ref.size == 4);
    assert(memcmp(ref.base, "defg", 4) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_fragment(&ctx, req, &ref);
    assert(ref.size == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_empty_query_and_fragment() {
    const char* req = "GET /about?# HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_request_get_abs_path(&ctx, req, &ref);
    assert(ref.size == 6);
    assert(memcmp(ref.base, "/about", 6) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_header(&ctx, req, "abc", 3, &ref);
    assert(ref.size == 4);
    assert(memcmp(ref.base, "defg", 4) == 0);

    qbuf_ref_reset(&ref);
    http_request_get_fragment(&ctx, req, &ref);
    assert(ref.size == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode() {
    test_req_decode1();
    test_req_decode_query();
    test_req_decode_header();
    test_req_decode_more_data();
    test_req_decode_no_content_len();
    test_req_decode_invalid_content_len();
    test_req_decode_fragment();
    test_req_decode_fragment2();
    test_req_decode_empty_fragment();
    test_req_decode_empty_query();
    test_req_decode_empty_query_and_fragment();
}
