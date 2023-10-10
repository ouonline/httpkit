static const char* g_retcode_str[] = {
    "OK",

    "out of mmeory",
    "invalid header",
    "unsupported or invalid status code",
    "invalid content length",
    "more data required",
    "invalid url",

    "invalid response line",
};

const char* http_get_retcode_str(unsigned rc) {
    return g_retcode_str[rc];
}
