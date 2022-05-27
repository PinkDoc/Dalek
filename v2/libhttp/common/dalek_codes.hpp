#pragma once

namespace http {

constexpr int HTTP_VERSION_9 = 9;
constexpr int HTTP_VERSION_10 = 1000;
constexpr int HTTP_VERSION_11 = 1001;
constexpr int HTTP_VERSION_20 = 2000;

constexpr int HTTP_UNKNOWN = 0X00000001;
constexpr int HTTP_GET = 0x00000002;
constexpr int HTTP_HEAD = 0x00000004;
constexpr int HTTP_POST = 0x00000008;
constexpr int HTTP_PUT = 0x00000010;
constexpr int HTTP_DELETE = 0x00000020;
constexpr int HTTP_OPTIONS = 0x00000200;
constexpr int HTTP_PATCH = 0x00000400;
constexpr int HTTP_TRACE = 0x00000800;
constexpr int HTTP_CONNECT = 0x00001000;

constexpr int HTTP_CONTINUE = 100;
constexpr int HTTP_SWITCHING_PROTOCOLS = 101;

constexpr int HTTP_OK = 200;
constexpr int HTTP_CREATED = 201;
constexpr int HTTP_ACCEPTED = 202;
constexpr int HTTP_NO_CONTENT = 204;
constexpr int HTTP_PARTIAL_CONTENT = 206;

constexpr int HTTP_MULITPLE_CHOICES = 300;
constexpr int HTTP_MOVED_PERMANENTLY = 301;
constexpr int HTTP_FOUND = 302;
constexpr int HTTP_SEE_OTHER = 303;
constexpr int HTTP_NOT_MODIFIED = 304;

constexpr int HTTP_BAD_REQUEST = 400;
constexpr int HTTP_UNAUTHORIZED = 401;
constexpr int HTTP_FORBINDDEN = 403;
constexpr int HTTP_NOT_FOUND = 404;
constexpr int HTTP_NOT_ALLOWED = 405;
constexpr int HTTP_REQUEST_TIME_OUT = 408;

constexpr int HTTP_INTERNAL_SERVER_ERROR = 500;
constexpr int HTTP_NOT_IMPLEMENTED = 501;
constexpr int HTTP_BAD_GATEWAY = 502;
constexpr int HTTP_SERVICE_UNAVAILABLE = 503;
constexpr int HTTP_GATEWAY_TIME_OUT = 504;
constexpr int HTTP_VERSION_NOT_SUPPORT = 505;
constexpr int HTTP_INSUFFICIENT_STORAGE = 507;

constexpr int HTTP_GIZP_BUFFERED = 0X20;
// TODO (pink)
constexpr int HTTP_SSL_BUFFERED = 0x01;

enum {
    DALEK_HTTP_PARSE_INVALID_METHOD,
    DALEK_HTTP_PARSE_INVALID_REQEUST,
    DALEK_HTTP_PARSE_INVALID_HTTP_VERSION,
    DALEK_HTTP_PARSE_INVALID_HEAD,
    DALEK_HTTP_HTTP2_PARSE_INVALID_FRAME,
    DALEK_HTTP_PARSE_HEADER_DONE,
    DALEK_AGAIN,
    DALEK_OK
};

// The http handle state
// init request -> reading -> process [handle uri, handle heads...] -> writing -> close/keepalive -> reading ...
enum http_state {
    HTTP_INIT_REQUEST_STATE = 0,
    HTTP_READING_REQUEST_STATE,
    HTTP_PROCESS_REQUEST_STATE,

    HTTP_CONNECT_UPSTREAM_STATE,
    HTTP_WRITING_UPSTREAM_STATE,
    HTTP_READING_UPSTREAM_STATE,

    HTTP_WRITING_REQUEST_STATE,
    HTTP_CLOSE_STATE,
    HTTP_KEEPALIVE_STATE
};
} // namespace http