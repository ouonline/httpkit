#undef NDEBUG
#include <assert.h>

#include "httpkit/http_common.h"
#include "httpkit/http_response_decode.h"

void test_res_decode1() {
    const char* res = "HTTP/1.1 404 Not Found\r\n\r\n";

    struct http_response_decode_context ctx;
    http_response_decode_context_init(&ctx);

    int rc = http_response_decode(&ctx, res, strlen(res));
    assert(rc == HRC_OK);

    unsigned int code = http_response_get_status_code(&ctx);
    assert(code == 404);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_response_get_status_text(&ctx, &ref);
    assert(ref.size == 9);
    assert(memcmp(ref.base, "Not Found", 9) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_version(&ctx, &ref);
    assert(ref.size == 8);
    assert(memcmp(ref.base, "HTTP/1.1", 8) == 0);

    assert(http_response_get_size(&ctx) == strlen(res));

    http_response_decode_context_destroy(&ctx);
}

void test_res_decode_header() {
    const char* res = "HTTP/1.1 200 OK\r\nfoo  : bar\r\nContent-Length: 0\r\n\r\n";

    struct http_response_decode_context ctx;
    http_response_decode_context_init(&ctx);

    int rc = http_response_decode(&ctx, res, strlen(res));
    assert(rc == HRC_OK);

    unsigned int code = http_response_get_status_code(&ctx);
    assert(code == 200);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_response_get_status_text(&ctx, &ref);
    assert(ref.size == 2);
    assert(memcmp(ref.base, "OK", 2) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_header(&ctx, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    assert(http_response_get_size(&ctx) == strlen(res));

    http_response_decode_context_destroy(&ctx);
}

void test_res_decode_more_data() {
    const char* res1 = "HTTP/1.1 20";
    const char* res2 = "HTTP/1.1 200 OK\r\nfo";
    const char* res3 = "HTTP/1.1 200 OK\r\nfoo: bar\r";
    const char* res = "HTTP/1.1 200 OK\r\nfoo: bar\r\nContent-Length: 0\r\n\r\n";

    struct http_response_decode_context ctx;
    http_response_decode_context_init(&ctx);

    int rc = http_response_decode(&ctx, res1, strlen(res1));
    assert(rc == HRC_MORE_DATA);

    rc = http_response_decode(&ctx, res2, strlen(res2));
    assert(rc == HRC_MORE_DATA);

    rc = http_response_decode(&ctx, res3, strlen(res3));
    assert(rc == HRC_MORE_DATA);

    rc = http_response_decode(&ctx, res, strlen(res));
    assert(rc == HRC_OK);

    unsigned int code = http_response_get_status_code(&ctx);
    assert(code == 200);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_response_get_status_text(&ctx, &ref);
    assert(ref.size == 2);
    assert(memcmp(ref.base, "OK", 2) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_header(&ctx, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    assert(http_response_get_size(&ctx) == strlen(res));

    http_response_decode_context_destroy(&ctx);
}

void test_res_decode_content() {
    const char* res = "HTTP/1.1 200 OK\r\nfoo  : bar\r\nContent-Length: 8\r\n\r\nouonline";

    struct http_response_decode_context ctx;
    http_response_decode_context_init(&ctx);

    int rc = http_response_decode(&ctx, res, strlen(res));
    assert(rc == HRC_OK);

    unsigned int code = http_response_get_status_code(&ctx);
    assert(code == 200);

    struct qbuf_ref ref;
    qbuf_ref_reset(&ref);
    http_response_get_status_text(&ctx, &ref);
    assert(ref.size == 2);
    assert(memcmp(ref.base, "OK", 2) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_header(&ctx, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_content(&ctx, &ref);
    assert(ref.size == 8);
    assert(memcmp(ref.base, "ouonline", 8) == 0);

    assert(http_response_get_size(&ctx) == strlen(res));

    http_response_decode_context_destroy(&ctx);
}

void test_res_decode() {
    test_res_decode1();
    test_res_decode_header();
    test_res_decode_more_data();
    test_res_decode_content();
}
