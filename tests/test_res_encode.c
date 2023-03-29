#undef NDEBUG
#include <assert.h>

#include "httpkit/http_common.h"
#include "httpkit/http_response_encode.h"
#include "httpkit/http_utils.h"
#include "test_utils.h"

void test_res_encode1() {
    struct qbuf_ref version = {.base = "HTTP/1.1", .size = 8};

    const struct http_response_status* st = http_response_status_lookup(404);
    assert(st != NULL);

    struct qbuf res;

    qbuf_init(&res);
    int rc = http_response_encode_head(st->code, st->text_str, st->text_len, NULL, 0, &res);
    assert(rc == HRC_OK);
    const char* expected = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
    rc = http_response_encode_head(st->code, st->text_str, st->text_len, NULL, 123, &res);
    assert(rc == HRC_OK);
    expected = "HTTP/1.1 404 Not Found\r\nContent-Length: 123\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

void test_res_encode() {
    test_res_encode1();
}
