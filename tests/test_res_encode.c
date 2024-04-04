#undef NDEBUG
#include <assert.h>

#include "httpkit/http_response_encode.h"
#include "httpkit/http_utils.h"
#include <string.h>

void test_res_encode1() {
    const struct http_response_status* st = http_response_status_lookup(404);
    assert(st != NULL);

    struct qbuf res;
    qbuf_init(&res);

    int rc = http_response_encode_status_line(&res, st->code, st->text_str, st->text_len);
    assert(rc == HRC_OK);
    rc = http_response_encode_head_end(&res);
    assert(rc == HRC_OK);

    const char* expected = "HTTP/1.1 404 Not Found\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

void test_res_encode() {
    test_res_encode1();
}
