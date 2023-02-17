#include <stdio.h>

void test_req_encode();
void test_req_decode();
void test_res_encode();
void test_res_decode();
void test_header_encode();
void test_header_decode();
void test_url_decode();

int main(void) {
    test_req_encode();
    test_req_decode();
    test_res_encode();
    test_res_decode();
    test_header_encode();
    test_header_decode();
    test_url_decode();
    printf("All tests were passed.\n");
    return 0;
}
