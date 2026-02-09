#ifndef HTTP_AUX_HPP
#define HTTP_AUX_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_http_client.h"
#pragma GCC diagnostic pop
#include <string>

namespace http {
namespace ContentType {
extern const char *key;
extern const char *json;
extern const char *url;
}; // namespace ContentType

/// HTTP codes see RFC7231
enum code {
    FAIL = -1,

    ERR_BASE = (0x7000), /*!< Starting number of HTTP error codes */
    ERR_MAX_REDIRECT =
        (ERR_BASE + 1), /*!< The error exceeds the number of HTTP redirects */
    ERR_CONNECT = (ERR_BASE + 2),    /*!< Error open the HTTP connection */
    ERR_WRITE_DATA = (ERR_BASE + 3), /*!< Error write HTTP data */
    ERR_FETCH_HEADER =
        (ERR_BASE + 4), /*!< Error read HTTP header from server */
    ERR_INVALID_TRANSPORT =
        (ERR_BASE +
         5), /*!< There are no transport support for the input scheme */
    ERR_CONNECTING =
        (ERR_BASE + 6), /*!< HTTP connection hasn't been established yet */
    ERR_EAGAIN = (ERR_BASE + 7), /*!< Mapping of errno EAGAIN to esp_err_t */
    ERR_CONNECTION_CLOSED =
        (ERR_BASE + 8), /*!< Read FIN from peer and the connection closed */
    ERR_NO_PAYLOAD = (ERR_BASE + 9),

    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101,
    PROCESSING = 102,
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NON_AUTHORITATIVE_INFORMATION = 203,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    PARTIAL_CONTENT = 206,
    MULTI_STATUS = 207,
    ALREADY_REPORTED = 208,
    IM_USED = 226,
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    USE_PROXY = 305,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    PAYMENT_REQUIRED = 402,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    PROXY_AUTHENTICATION_REQUIRED = 407,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PRECONDITION_FAILED = 412,
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    RANGE_NOT_SATISFIABLE = 416,
    EXPECTATION_FAILED = 417,
    MISDIRECTED_REQUEST = 421,
    UNPROCESSABLE_ENTITY = 422,
    LOCKED = 423,
    FAILED_DEPENDENCY = 424,
    UPGRADE_REQUIRED = 426,
    PRECONDITION_REQUIRED = 428,
    TOO_MANY_REQUESTS = 429,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    VARIANT_ALSO_NEGOTIATES = 506,
    INSUFFICIENT_STORAGE = 507,
    LOOP_DETECTED = 508,
    NOT_EXTENDED = 510,
    NETWORK_AUTHENTICATION_REQUIRED = 511
};

static inline bool is_client_error(int code) {
    return 400 <= code && code < 500;
}
static inline bool is_server_error(int code) {
    return 500 <= code && code < 600;
}
static inline bool is_http_error(int code) { return 400 <= code && code < 600; }
static inline bool is_connection_error(int code) {
    return ERR_BASE < code && code != ERR_NO_PAYLOAD;
}
static inline bool is_request_successful(int code) {
    return code == code::OK || code == code::NO_CONTENT ||
           code == ERR_NO_PAYLOAD;
}
static inline bool is_request_failed(int code) {
    return !is_request_successful(code);
}
static inline bool is_unauthorized(int code) {
    return code == code::UNAUTHORIZED;
}
}; // namespace http

#endif
