#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "httpkit/http_header.h"
#include "httpkit/http_common.h"
#include "cutils/str_utils.h"
#include <string.h>

int http_header_init(struct http_header* header) {
    header->content_len = 0;
    return 0;
}

void http_header_destroy(struct http_header* header) {
    header->content_len = 0;
}

typedef void (*parse_header_field_func)(const char* key, unsigned int keylen,
                                        const char* value, unsigned int valuelen,
                                        struct http_header* header);

static void get_content_length_func(const char* key, unsigned int keylen,
                                    const char* value, unsigned int valuelen,
                                    struct http_header* header) {
    (void)key;
    (void)keylen;

    header->content_len = 0;
    ndec2int(value, valuelen, (int*)(&header->content_len));
}

static const struct header_field_handler {
    const char* key;
    unsigned int keylen;
    parse_header_field_func func;
} g_header_filed_func_list[] = {
    {"Content-Length", 14, get_content_length_func},
    {NULL, 0, NULL},
};

static int parse_header_field(const char* key, unsigned int klen,
                              const char* value, unsigned int vlen,
                              const struct header_field_handler* handler,
                              struct http_header* header) {
    if (klen != handler->keylen) {
        return HRE_EMPTY;
    }

    if (memcmp(key, handler->key, klen) != 0) {
        return HRE_EMPTY;
    }

    handler->func(key, klen, value, vlen, header);
    return HRE_SUCCESS;
}

static void parse_all_header_fields(const char* key, unsigned int klen,
                                    const char* value, unsigned int vlen,
                                    struct http_header* header) {
    int i;
    for (i = 0; g_header_filed_func_list[i].key; ++i) {
        if (parse_header_field(key, klen, value, vlen,
                               &(g_header_filed_func_list[i]),
                               header) == 0) {
            return;
        }
    }
}

static void parse_header_line(const char* data, unsigned int len,
                              parse_header_field_func parse_func,
                              struct http_header* header) {
    unsigned int keylen = 0;
    const char* end = data + len;
    const char* cursor = (const char*)memmem(data, len, ":", 1);
    if (!cursor) {
        return;
    }
    keylen = cursor - data;

    while (1) {
        ++cursor;
        if (cursor >= end) {
            return; /* nothing */
        }
        if (*cursor != ' ') {
            break;
        }
    }

    parse_func(data, keylen, cursor, len - (cursor - data), header);
}

static void parse_header(const char* data, unsigned int len,
                         parse_header_field_func parse_func,
                         struct http_header* header) {
    while (len > 0) {
        const char* eol = (const char*)memmem(data, len, "\r\n", 2);
        if (!eol) {
            return;
        }

        parse_header_line(data, eol - data, parse_func, header);

        len -= (eol - data + 2);
        data = eol + 2;
    }
}

void http_header_decode(struct http_header* header,
                        const char* data, unsigned int len) {
    parse_header(data, len, parse_all_header_fields, header);
}
