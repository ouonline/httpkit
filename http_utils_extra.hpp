#ifndef __HTTPKIT_HTTP_UTILS_EXTRA_HPP__
#define __HTTPKIT_HTTP_UTILS_EXTRA_HPP__

#include "http_request.h"
#include "http_response.h"

class HttpRequestGuard final {
public:
    HttpRequestGuard(HttpRequest* req) : m_req(req) {}
    ~HttpRequestGuard() {
        http_request_destroy(m_req);
    }
private:
    HttpRequest* m_req;
};

class HttpResponseGuard final {
public:
    HttpResponseGuard(HttpResponse* res) : m_res(res) {}
    ~HttpResponseGuard() {
        http_response_destroy(m_res);
    }
private:
    HttpResponse* m_res;
};

#endif
