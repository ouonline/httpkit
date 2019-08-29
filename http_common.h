#ifndef __HTTP_COMMON_H__
#define __HTTP_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

enum {
    HTTP_STATUS_100 = 0,
    HTTP_STATUS_101,

    HTTP_STATUS_200,
    HTTP_STATUS_201,
    HTTP_STATUS_202,
    HTTP_STATUS_203,
    HTTP_STATUS_204,
    HTTP_STATUS_205,
    HTTP_STATUS_206,

    HTTP_STATUS_300,
    HTTP_STATUS_301,
    HTTP_STATUS_302,
    HTTP_STATUS_303,
    HTTP_STATUS_304,
    HTTP_STATUS_305,
    HTTP_STATUS_307,

    HTTP_STATUS_400,
    HTTP_STATUS_401,
    HTTP_STATUS_402,
    HTTP_STATUS_403,
    HTTP_STATUS_404,
    HTTP_STATUS_405,
    HTTP_STATUS_406,
    HTTP_STATUS_407,
    HTTP_STATUS_408,
    HTTP_STATUS_409,
    HTTP_STATUS_410,
    HTTP_STATUS_411,
    HTTP_STATUS_412,
    HTTP_STATUS_413,
    HTTP_STATUS_414,
    HTTP_STATUS_415,
    HTTP_STATUS_416,
    HTTP_STATUS_417,

    HTTP_STATUS_500,
    HTTP_STATUS_501,
    HTTP_STATUS_502,
    HTTP_STATUS_503,
    HTTP_STATUS_504,
    HTTP_STATUS_505,

    HTTP_STATUS_UNSUPPORTED,
};

/* error code */
#define HRE_SUCCESS      0
#define HRE_NOMEM        (-1)  /* out of memory */
#define HRE_REQLINE      (-2)  /* request line error */
#define HRE_REQMETHOD    (-3)  /* unknown/unsupported request method */
#define HRE_URLDECODE    (-4)  /* url decoding error */
#define HRE_HEADER       (-5)  /* invalid request header */
#define HRE_EMPTY        (-6)  /* empty argument/result */
#define HRE_HTTP_STATUS  (-7)  /* invalid http status */
#define HRE_CONTENT_TYPE (-8)  /* invalid content type */
#define HRE_CONTENT_SIZE (-9)  /* content size mismatch */
#define HRE_BUFSIZE      (-10) /* invalid buffer size */
#define HRE_RESLINE      (-11) /* response status line error */

#ifdef __cplusplus
}
#endif

#endif
