#undef NDEBUG
#include <assert.h>

#include "httpkit/http_retcode.h"
#include "httpkit/http_utils.h"
#include <string.h>

void test_url_decode() {
    const char* str = "%23from+ou%25%21online";
    const char* expected = "#from ou%!online";
    struct qbuf res;
    qbuf_init(&res);
    int rc = http_url_decode(str, strlen(str), &res);
    assert(rc == HRC_OK);
    assert(qbuf_size(&res) == strlen(expected));
    assert(memcmp(qbuf_data(&res), expected, qbuf_size(&res)) == 0);
}
