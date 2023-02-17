#undef NDEBUG
#include <assert.h>

#include "httpkit/http_common.h"
#include "httpkit/http_response_encode.h"
#include "test_utils.h"

void test_res_encode1() {
    struct qbuf_ref version = {.base = "HTTP/1.1", .size = 8};

    struct qbuf res;
    qbuf_init(&res);
    int rc = http_response_encode(200, NULL, NULL, 0, &res);
    assert(rc == HRC_OK);

    const char* expected = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

void test_res_encode() {
    test_res_encode1();
}
