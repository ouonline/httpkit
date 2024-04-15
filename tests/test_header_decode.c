#undef NDEBUG
#include <assert.h>
#include <string.h>

#include "../src/misc.h"
#include "httpkit/http_retcode.h"
#include "../src/http_header_decode.h"

void test_header_decode1() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;

    const char* hdr = "ou: ouonline\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct kvpair* h = kvpair_vector_lookup(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->value.off;
    unsigned long vlen = h->value.len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_multi() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;

    const char* hdr = "foo: bar\r\nbar: baz\r\nfoobar: ouonline\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct kvpair* h = kvpair_vector_lookup(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    const char* value = hdr + h->value.off;
    unsigned long vlen = h->value.len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

    h = kvpair_vector_lookup(&hdr_list, hdr, "bar", 3);
    assert(h != NULL);
    value = hdr + h->value.off;
    vlen = h->value.len;
    assert(vlen == 3);
    assert(memcmp(value, "baz", 3) == 0);

    h = kvpair_vector_lookup(&hdr_list, hdr, "foobar", 6);
    assert(h != NULL);
    value = hdr + h->value.off;
    vlen = h->value.len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_spaces_at_the_beginning() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;

    const char* hdr = "  \t   ou:ouonline\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct kvpair* h = kvpair_vector_lookup(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->value.off;
    unsigned long vlen = h->value.len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_spaces_between_key_and_colon() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;

    const char* hdr = "  \t   ou \t\t\t :ouonline\r\nfoo:bar\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct kvpair* h = kvpair_vector_lookup(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->value.off;
    unsigned long vlen = h->value.len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    h = kvpair_vector_lookup(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    value = hdr + h->value.off;
    vlen = h->value.len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_spaces_between_colon_and_value() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;

    const char* hdr = "  \t   ou \t\t\t :  \t  \t ouonline\r\nfoo:\t\t\tbar\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct kvpair* h = kvpair_vector_lookup(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->value.off;
    unsigned long vlen = h->value.len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", 8) == 0);

    h = kvpair_vector_lookup(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    value = hdr + h->value.off;
    vlen = h->value.len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", 3) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_spaces_after_value() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;

    const char* hdr = "  \t   ou \t\t\t :  \t  \t ouonline   \t\r\nfoo:\t\t\tbar\t\t\t \t\t\t\r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);
    assert(offset == strlen(hdr));

    struct kvpair* h = kvpair_vector_lookup(&hdr_list, hdr, "ou", 2);
    assert(h != NULL);
    const char* value = hdr + h->value.off;
    unsigned long vlen = h->value.len;
    assert(vlen == 8);
    assert(memcmp(value, "ouonline", vlen) == 0);

    h = kvpair_vector_lookup(&hdr_list, hdr, "foo", 3);
    assert(h != NULL);
    value = hdr + h->value.off;
    vlen = h->value.len;
    assert(vlen == 3);
    assert(memcmp(value, "bar", vlen) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_content_len() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long content_len = 0;
    unsigned long offset = 0;

    const char* hdr = "Content-Length: 8  \t \r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK); /* Content-Length is required */

    set_content_len(hdr, &hdr_list, &content_len);
    assert(content_len == 8);

    cvector_destroy(&hdr_list);
}

void test_header_decode_spaces_in_value() {
    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;
    const char* hdr = "Accept-Encoding: gzip, json, deflate \r\n\r\n";
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);

    struct kvpair* item = kvpair_vector_lookup(&hdr_list, hdr, "Accept-Encoding", 15);
    assert(item != NULL);
    assert(item->value.len == 19);
    assert(memcmp(hdr + item->value.off, "gzip, json, deflate", item->value.len) == 0);

    cvector_destroy(&hdr_list);
}

void test_header_decode_empty() {
    const char* hdr = "\r\n";

    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);

    cvector_destroy(&hdr_list);
}

void test_header_decode_empty2() {
    const char* hdr = "\r\n";

    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_OK);

    cvector_destroy(&hdr_list);
}

void test_header_decode_more_data() {
    const char* hdr = "\r";

    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_MORE_DATA);

    cvector_destroy(&hdr_list);
}

void test_header_decode_more_data2() {
    const char* hdr = "foo:bar\r\n";

    struct cvector hdr_list; /* element type is struct kvpair */
    cvector_init(&hdr_list, sizeof(struct kvpair));

    unsigned long offset = 0;
    int rc = http_header_decode(hdr, strlen(hdr), hdr, &hdr_list, &offset);
    assert(rc == HRC_MORE_DATA);

    cvector_destroy(&hdr_list);
}

void test_header_decode() {
    test_header_decode1();
    test_header_decode_multi();
    test_header_decode_spaces_at_the_beginning();
    test_header_decode_spaces_between_key_and_colon();
    test_header_decode_spaces_between_colon_and_value();
    test_header_decode_spaces_after_value();
    test_header_decode_content_len();
    test_header_decode_spaces_in_value();
    test_header_decode_empty();
    test_header_decode_empty2();
    test_header_decode_more_data();
    test_header_decode_more_data2();
}
