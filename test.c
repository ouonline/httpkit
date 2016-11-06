#include "http_request.h"
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
    if (http_request_init(&req) != 0) {
        fprintf(stderr, "http_request_init error\n");
        return;
    }

    int err = http_request_parse(&req, get_req, strlen(get_req));
    if (err) {
        fprintf(stderr, "parse error: %d\n", err);
        return;
    }

    printf("-----------------\n");
    http_request_for_each_option(&req, NULL, print_option_func);
    printf("-----------------\n");
    printf("get content type = %d\n", http_request_get_content_type(&req));

    char buf[4096];
    http_pack_response(HTTP_STATUS_200, HTTP_CONTENT_TYPE_PLAIN,
                       "abc", 3, buf, sizeof(buf));

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

    int err = http_request_parse(&req, post_req, strlen(post_req));
    if (err) {
        fprintf(stderr, "parse error: %d\n", err);
        return;
    }

    printf("-----------------\n");
    http_request_for_each_option(&req, NULL, print_option_func);
    printf("-----------------\n");

    printf("get content type = %d\n", http_request_get_content_type(&req));

    http_request_destroy(&req);
}

int main(void) {
    test1();
    test2();
    return 0;
}
