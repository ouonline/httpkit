#ifndef __HTTPKIT_HTTP_RETCODE_H__
#define __HTTPKIT_HTTP_RETCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* returned values */
enum {
    HRC_OK = 0,

    /* common */

    HRC_NOMEM,
    HRC_HEADER,  /* invalid header */
    HRC_STATUS_CODE, /* unsupported or invalid status code */
    HRC_CONTENTLEN, /* invalid content length */
    HRC_MORE_DATA, /* more data required */
    HRC_URL, /* invalid url */

    /* responses */

    HRC_RES_LINE, /* invalid response line */
};

const char* http_get_retcode_str(unsigned rc);

#ifdef __cplusplus
}
#endif

#endif
