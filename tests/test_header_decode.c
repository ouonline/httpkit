#undef NDEBUG
#include <assert.h>
#include <string.h>

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

void test_header_decode_multi() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "foo: bar\r\nbar: baz\r\nfoobar: ouonline\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct qbuf_ol* h = http_kv_ol_list_get(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    const char* value = hdr + h->off;
    unsigned long vlen = h->len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

    h = http_kv_ol_list_get(&hdr_list, hdr, "bar", 3);
    assert(h != NULL);
    value = hdr + h->off;
    vlen = h->len;
    assert(vlen == 3);
    assert(memcmp(value, "baz", 3) == 0);

    h = http_kv_ol_list_get(&hdr_list, hdr, "foobar", 6);
    assert(h != NULL);
    value = hdr + h->off;
    vlen = h->len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_spaces_at_the_beginning() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "  \t   ou:ouonline\r\n\r\n";
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

void test_header_decode_spaces_between_key_and_colon() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "  \t   ou \t\t\t :ouonline\r\nfoo:bar\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct qbuf_ol* h = http_kv_ol_list_get(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->off;
    unsigned long vlen = h->len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    h = http_kv_ol_list_get(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    value = hdr + h->off;
    vlen = h->len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_spaces_between_colon_and_value() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "  \t   ou \t\t\t :  \t  \t ouonline\r\nfoo:\t\t\tbar\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct qbuf_ol* h = http_kv_ol_list_get(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->off;
    unsigned long vlen = h->len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    h = http_kv_ol_list_get(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    value = hdr + h->off;
    vlen = h->len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_spaces_after_value() {
    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;

    const char* hdr = "  \t   ou \t\t\t :  \t  \t ouonline   \t\r\nfoo:\t\t\tbar\t\t\t \t\t\t\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct qbuf_ol* h = http_kv_ol_list_get(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->off;
    unsigned long vlen = h->len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    h = http_kv_ol_list_get(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    value = hdr + h->off;
    vlen = h->len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

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

void test_header_decode_empty() {
    const char* hdr = "\r\n";

    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_empty2() {
    const char* hdr = "\r\n";

    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_more_data() {
    const char* hdr = "\r";

    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_MORE_DATA);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode_more_data2() {
    const char* hdr = "foo:bar\r\n";

    struct http_kv_ol_list hdr_list;
    http_kv_ol_list_init(&hdr_list);

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_MORE_DATA);

    http_kv_ol_list_destroy(&hdr_list);
}

void test_header_decode() {
    test_header_decode1();
    test_header_decode_multi();
    test_header_decode_spaces_at_the_beginning();
    test_header_decode_spaces_between_key_and_colon();
    test_header_decode_spaces_between_colon_and_value();
    test_header_decode_spaces_after_value();
    test_header_decode_content_len();
    test_header_decode_empty();
    test_header_decode_empty2();
    test_header_decode_more_data();
    test_header_decode_more_data2();
}
