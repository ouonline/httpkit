#undef NDEBUG
#include <assert.h>

#include "httpkit/http_retcode.h"
#include "httpkit/http_utils.h"
#include <string.h>

static void test_url_decode1() {
    const char* str = "%23from+ou%25%21online";
    const char* expected = "#from ou%!online";
    struct qbuf res;
    qbuf_init(&res);
    int rc = http_url_decode(str, strlen(str), &res);
    assert(rc == HRC_OK);
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);
    qbuf_destroy(&res);
}

static void test_url_decode_invalid_escape() {
    const char* strs[] = {"%zz", "%2G", "abc%", "abc%2"};
    const unsigned int nr_strs = sizeof(strs) / sizeof(strs[0]);

    for (unsigned int i = 0; i < nr_strs; ++i) {
        struct qbuf res;
        qbuf_init(&res);

        int rc = http_url_decode(strs[i], strlen(strs[i]), &res);
        assert(rc == HRC_URL);
        assert(qbuf_size(&res) == 0);

        qbuf_destroy(&res);
    }
}

void test_url_decode() {
    test_url_decode1();
    test_url_decode_invalid_escape();
}
