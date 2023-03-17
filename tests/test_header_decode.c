#undef NDEBUG
#include <assert.h>
#include <string.h>
#include <limits.h>

#include "../src/misc.h"
#include "httpkit/http_common.h"
#include "../src/http_header_decode.h"

void test_header_decode1() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "ou: ouonline\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct qbuf_ol* h = http_kv_ol_list_get(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->off;
    unsigned long vlen = h->len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_with_multiple_spaces() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "ou   :    ouonline\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct qbuf_ol* h = http_kv_ol_list_get(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->off;
    unsigned long vlen = h->len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_content_len() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long content_len = 0;
    unsigned long offset = 0;

    const char* hdr = "Content-Length: 8\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK); /* Content-Length is required */

    set_content_len(hdr, &hdr_list, &content_len);
    assert(content_len == 8);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode() {
    test_header_decode1();
    test_header_decode_with_multiple_spaces();
    test_header_decode_content_len();
}
