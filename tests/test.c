#include "httpkit/http_common.h"
#include "httpkit/http_request.h"
#include "httpkit/http_response.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

static int print_option_func(void* nil, const char* k, unsigned int klen,
                             const char* v, unsigned int vlen) {
    (void)nil;

    write(1, k, klen);
    write(1, " -> ", 4);
    write(1, v, vlen);
    write(1, "\n", 1);
    return 0;
}

static void test1(void) {
    const char* get_req = "GET /index.html?k+%251=v1&k2=v2&k3=v3 HTTP/1.1\r\n"
"Host: 127.0.0.1:55555\r\n"
"Connection: keep-alive\r\n"
"Cache-Control: max-age=0\r\n"
"Upgrade-Insecure-Requests: 1\r\n"
"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.116 Safari/537.36\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
"Accept-Encoding: gzip, deflate, sdch\r\n"
"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n";

    struct http_request req;
    if (http_request_init(&req) != HRE_SUCCESS) {
        fprintf(stderr, "http_request_init error\n");
        return;
    }

    if (http_request_decode(&req, get_req, strlen(get_req)) != HRE_SUCCESS) {
        fprintf(stderr, "http_request_decode failed.\n");
        return;
    }

    printf("-----------------\n");
    http_request_for_each_option(&req, NULL, print_option_func);

    http_request_destroy(&req);
}

static void test2(void) {
    const char* post_req = "POST / HTTP/1.1\r\n"
"Host: 127.0.0.1:55555\r\n"
"Connection: keep-alive\r\n"
"Content-Length: 35\r\n"
"Cache-Control: max-age=0\r\n"
"Origin: null\r\n"
"Upgrade-Insecure-Requests: 1\r\n"
"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.116 Safari/537.36\r\n"
"Content-Type: application/x-www-form-urlencoded\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
"Accept-Encoding: gzip, deflate\r\n"
"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"
"text=text1&text=text2&submit=submit";

    struct http_request req;
    if (http_request_init(&req) != 0) {
        fprintf(stderr, "http_request_init error\n");
        return;
    }

    if (http_request_decode(&req, post_req, strlen(post_req)) != HRE_SUCCESS) {
        fprintf(stderr, "http_request_decode failed.\n");
        return;
    }

    printf("-----------------\n");
    http_request_for_each_option(&req, NULL, print_option_func);

    http_request_destroy(&req);
}

static void test_http_response_encode(void) {
    struct http_response res;
    const char* data = "{\"status\": \"ok\"}";

    http_response_init(&res);
    if (http_response_encode(&res, HTTP_STATUS_200, data, strlen(data)) == HRE_SUCCESS) {
        struct qbuf_ref ref;
        http_response_get_packet(&res, &ref);
        write(1, ref.base, ref.size);
        printf("\n");
    } else {
        fprintf(stderr, "pack response failed.\n");
    }
    http_response_destroy(&res);
}

static void test_http_request_encode(void) {
    struct http_request req;
    if (http_request_init(&req) != HRE_SUCCESS) {
        fprintf(stderr, "http_request_init falied.\n");
        return;
    }

    http_request_set_method(&req, HTTP_REQUEST_METHOD_GET);

    if (http_request_encode(&req, NULL, 0) != HRE_SUCCESS) {
        fprintf(stderr, "http_request_encode failed.\n");
        goto err;
    }

    struct qbuf_ref ref;
    http_request_get_packet(&req, &ref);
    write(1, ref.base, ref.size);
    printf("\n");

err:
    http_request_destroy(&req);
}

int main(void) {
    test1();
    test2();
    test_http_response_encode();
    test_http_request_encode();
    return 0;
}
