#undef NDEBUG
#include <assert.h>

#include "../src/http_header_encode.h"
#include "httpkit/http_retcode.h"
#include <string.h>

static void test_header_encode1() {
    struct qbuf res;

    qbuf_init(&res);
    int rc = http_header_encode(&res, "foo", 3, "bar", 3);
    assert(rc == HRC_OK);
    const char* expected = "foo:bar\r\n";
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);

    qbuf_destroy(&res);
}

void test_header_encode() {
    test_header_encode1();
}
