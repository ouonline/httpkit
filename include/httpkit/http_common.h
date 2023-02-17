#ifndef __HTTPKIT_HTTP_COMMON_H__
#define __HTTPKIT_HTTP_COMMON_H__

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
    HRC_URLDECODE, /* decode url failed */

    /* requests */

    HRC_REQLINE, /* request line error */
    HRC_REQMETHOD, /* invalid or unsupported request method */
    HRC_REQOPTION, /* invalid option */

    /* responses */

    HRC_RESLINE, /* response line error */
};

#ifdef __cplusplus
}
#endif

#endif
