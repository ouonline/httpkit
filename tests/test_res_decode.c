#undef NDEBUG
#include <assert.h>

#include "httpkit/http_retcode.h"
#include "httpkit/http_response_decode.h"
#include "../src/def.h"
#include <string.h>

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
    http_response_get_status_text(&ctx, res, &ref);
    assert(ref.size == 9);
    assert(memcmp(ref.base, "Not Found", 9) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_version(&ctx, res, &ref);
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
    http_response_get_status_text(&ctx, res, &ref);
    assert(ref.size == 2);
    assert(memcmp(ref.base, "OK", 2) == 0);

    qbuf_ref_reset(&ref);
    http_response_find_header(&ctx, res, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_response_find_header(&ctx, res, "not-found", 9, &ref);
    assert(ref.base == NULL);
    assert(ref.size == 0);

    assert(http_response_get_size(&ctx) == strlen(res));

    http_response_decode_context_destroy(&ctx);
}

struct kvref {
    struct qbuf_ref key;
    struct qbuf_ref value;
};

/* returns 1 if found */
static int find_key(const struct kvref* res_list, unsigned int sz, const struct qbuf_ref* key) {
    for (unsigned int i = 0; i < sz; ++i) {
        const struct kvref* r = &res_list[i];
        if (key->size != r->key.size) {
            continue;
        }
        if (memcmp(r->key.base, key->base, key->size) == 0) {
            return 1;
        }
    }
    return 0;
}

void test_res_decode_header_iterate() {
    const struct kvref expected_result[] = {
        {{"foo", 3}, {"bar", 3}},
        {{"Content-Length", CONTENT_LENGTH_LEN}, {"0", 1}},
    };
    const unsigned int expected_sz = sizeof(expected_result) / sizeof(struct kvref);

    const char* res = "HTTP/1.1 200 OK\r\nfoo  : bar\r\nContent-Length: 0\r\n\r\n";

    struct http_response_decode_context ctx;
    http_response_decode_context_init(&ctx);

    int rc = http_response_decode(&ctx, res, strlen(res));
    assert(rc == HRC_OK);

    unsigned int nr_header = http_response_get_header_count(&ctx);
    assert(nr_header == expected_sz);

    unsigned int nr_found = 0;
    for (unsigned int i = 0; i < nr_header; ++i) {
        struct qbuf_ref key;
        http_response_get_header(&ctx, res, i, &key, NULL);
        nr_found += find_key(expected_result, expected_sz, &key);
    }
    assert(nr_found == nr_header);

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
    http_response_get_status_text(&ctx, res, &ref);
    assert(ref.size == 2);
    assert(memcmp(ref.base, "OK", 2) == 0);

    qbuf_ref_reset(&ref);
    http_response_find_header(&ctx, res, "foo", 3, &ref);
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
    http_response_get_status_text(&ctx, res, &ref);
    assert(ref.size == 2);
    assert(memcmp(ref.base, "OK", 2) == 0);

    qbuf_ref_reset(&ref);
    http_response_find_header(&ctx, res, "foo", 3, &ref);
    assert(ref.size == 3);
    assert(memcmp(ref.base, "bar", 3) == 0);

    qbuf_ref_reset(&ref);
    http_response_get_content(&ctx, res, &ref);
    assert(ref.size == 8);
    assert(memcmp(ref.base, "ouonline", 8) == 0);

    assert(http_response_get_size(&ctx) == strlen(res));

    http_response_decode_context_destroy(&ctx);
}

void test_res_decode() {
    test_res_decode1();
    test_res_decode_header();
    test_res_decode_header_iterate();
    test_res_decode_more_data();
    test_res_decode_content();
}
