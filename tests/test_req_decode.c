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

    struct offlen ref;
    http_request_get_method(&ctx, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "GET", 3) == 0);

    offlen_reset(&ref);
    http_request_get_abs_path(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "/about", 6) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "Content-Length", 14, &ref);
    assert(ref.len == 1);
    assert(memcmp(req + ref.off, "8", 1) == 0);

    offlen_reset(&ref);
    http_request_get_content(&ctx, &ref);
    assert(ref.len == 8);
    assert(memcmp(req + ref.off, "ouonline", 8) == 0);

    offlen_reset(&ref);
    http_request_get_version(&ctx, &ref);
    assert(ref.len == 8);
    assert(memcmp(req + ref.off, "HTTP/1.1", 8) == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_query() {
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nContent-Length: 0\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_find_query(&ctx, req, "foo", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "bar", 3) == 0);

    offlen_reset(&ref);
    http_request_find_query(&ctx, req, "bar", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "baz", 3) == 0);

    offlen_reset(&ref);
    http_request_find_query(&ctx, req, "unknown", 7, &ref);
    assert(ref.off == 0);
    assert(ref.len == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

struct kvref {
    struct qbuf_ref key;
    struct qbuf_ref value;
};

/* returns 1 if found */
static int find_key(const struct kvref* res_list, unsigned int sz, const void* base, const struct offlen* key) {
    for (unsigned int i = 0; i < sz; ++i) {
        const struct kvref* r = &res_list[i];
        if (key->len != r->key.size) {
            continue;
        }
        if (memcmp(r->key.base, (const char*)base + key->off, key->len) == 0) {
            return 1;
        }
    }
    return 0;
}

void test_req_decode_query_iterate() {
    const struct kvref expected_result[] = {
        {{"foo", 3}, {"bar", 3}},
        {{"bar", 3}, {"baz", 3}},
    };
    const unsigned int expected_sz = sizeof(expected_result) / sizeof(struct kvref);

    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: def\r\nkkk: lll\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    unsigned nr_query = http_request_get_query_count(&ctx);
    assert(nr_query == expected_sz);

    unsigned int nr_found = 0;
    for (unsigned int i = 0; i < nr_query; ++i) {
        struct offlen key;
        http_request_get_query(&ctx, i, &key, NULL);
        nr_found += find_key(expected_result, expected_sz, req, &key);
    }
    assert(nr_found == nr_query);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_header() {
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "abc", 3, &ref);
    assert(ref.len == 4);
    assert(memcmp(req + ref.off, "defg", 4) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "foo", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "bar", 3) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "not-found", 9, &ref);
    assert(ref.off == 0);
    assert(ref.len == 0);

    assert(http_request_get_size(&ctx) == strlen(req));

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_header_iterate() {
    const struct kvref expected_result[] = {
        {{"abc", 3}, {"def", 3}},
        {{"kkk", 3}, {"lll", 3}},
    };
    const unsigned int expected_sz = sizeof(expected_result) / sizeof(struct kvref);

    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: def\r\nkkk: lll\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    unsigned nr_header = http_request_get_header_count(&ctx);
    assert(nr_header == expected_sz);

    unsigned int nr_found = 0;
    for (unsigned int i = 0; i < nr_header; ++i) {
        struct offlen key;
        http_request_get_header(&ctx, i, &key, NULL);
        nr_found += find_key(expected_result, expected_sz, req, &key);
    }
    assert(nr_found == nr_header);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_more_data() {
    const char* req1 = "GET /about?foo=bar&ba";
    const char* req2 = "GET /about?foo=bar&bar=baz HTTP/1.1\r";
    const char* req3 = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\nContent-Length: 5\r\n\r\nhe";
    const char* req4 = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\nContent-Length: 5\r\n\r\nhell";
    const char* req = "GET /about?foo=bar&bar=baz HTTP/1.1\r\nabc: defg\r\nfoo: bar\r\nContent-Length: 5\r\n\r\nhello";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req1, strlen(req1));
    assert(rc == HRC_MORE_DATA);

    rc = http_request_decode(&ctx, req2, strlen(req2));
    assert(rc == HRC_MORE_DATA);
    assert(ctx.bytes_left == 0);

    rc = http_request_decode(&ctx, req3, strlen(req3));
    assert(rc == HRC_MORE_DATA);
    assert(ctx.bytes_left == 3);

    rc = http_request_decode(&ctx, req4, strlen(req4));
    assert(rc == HRC_MORE_DATA);
    assert(ctx.bytes_left == 1);

    rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "abc", 3, &ref);
    assert(ref.len == 4);
    assert(memcmp(req + ref.off, "defg", 4) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "foo", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "bar", 3) == 0);

    offlen_reset(&ref);
    http_request_find_query(&ctx, req, "foo", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "bar", 3) == 0);

    offlen_reset(&ref);
    http_request_find_query(&ctx, req, "bar", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "baz", 3) == 0);

    offlen_reset(&ref);
    http_request_get_content(&ctx, &ref);
    assert(ref.len == 5);
    assert(memcmp(req + ref.off, "hello", 5) == 0);

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

    struct offlen ref;
    offlen_reset(&ref);
    http_request_find_query(&ctx, req, "foo", 3, &ref);
    assert(ref.len == 3);
    assert(memcmp(req + ref.off, "bar", 3) == 0);

    offlen_reset(&ref);
    http_request_get_fragment(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "anchor", 6) == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_fragment2() {
    const char* req = "GET /about#anchor HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_get_abs_path(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "/about", 6) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "abc", 3, &ref);
    assert(ref.len == 4);
    assert(memcmp(req + ref.off, "defg", 4) == 0);

    offlen_reset(&ref);
    http_request_get_fragment(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "anchor", 6) == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_empty_fragment() {
    const char* req = "GET /about# HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_get_abs_path(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "/about", 6) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "abc", 3, &ref);
    assert(ref.len == 4);
    assert(memcmp(req + ref.off, "defg", 4) == 0);

    offlen_reset(&ref);
    http_request_get_fragment(&ctx, &ref);
    assert(ref.len == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_empty_query() {
    const char* req = "GET /about? HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_get_abs_path(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "/about", 6) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "abc", 3, &ref);
    assert(ref.len == 4);
    assert(memcmp(req + ref.off, "defg", 4) == 0);

    offlen_reset(&ref);
    http_request_get_fragment(&ctx, &ref);
    assert(ref.len == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode_empty_query_and_fragment() {
    const char* req = "GET /about?# HTTP/1.1\r\nabc: defg\r\n\r\n";

    struct http_request_decode_context ctx;
    http_request_decode_context_init(&ctx);

    int rc = http_request_decode(&ctx, req, strlen(req));
    assert(rc == HRC_OK);

    struct offlen ref;
    offlen_reset(&ref);
    http_request_get_abs_path(&ctx, &ref);
    assert(ref.len == 6);
    assert(memcmp(req + ref.off, "/about", 6) == 0);

    offlen_reset(&ref);
    http_request_find_header(&ctx, req, "abc", 3, &ref);
    assert(ref.len == 4);
    assert(memcmp(req + ref.off, "defg", 4) == 0);

    offlen_reset(&ref);
    http_request_get_fragment(&ctx, &ref);
    assert(ref.len == 0);

    http_request_decode_context_destroy(&ctx);
}

void test_req_decode() {
    test_req_decode1();
    test_req_decode_query();
    test_req_decode_query_iterate();
    test_req_decode_header();
    test_req_decode_header_iterate();
    test_req_decode_more_data();
    test_req_decode_no_content_len();
    test_req_decode_invalid_content_len();
    test_req_decode_fragment();
    test_req_decode_fragment2();
    test_req_decode_empty_fragment();
    test_req_decode_empty_query();
    test_req_decode_empty_query_and_fragment();
}
