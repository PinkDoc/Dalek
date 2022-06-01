#pragma once

#include "assert.hpp"
#include <string_view>

namespace lighthttp
{
enum
{
    InvalidRequest = -1,
    InvalidHead,
    Again,
    RequestLineOk, // Request parse ok
    HeadLineOk,    // One head
    HeadDone
};

// Method
constexpr int HTTP_UNKNOWN = 0X00000001;
constexpr int HTTP_GET     = 0x00000002;
constexpr int HTTP_HEAD    = 0x00000004;
constexpr int HTTP_POST    = 0x00000008;
constexpr int HTTP_PUT     = 0x00000010;
constexpr int HTTP_DELETE  = 0x00000020;
constexpr int HTTP_OPTIONS = 0x00000200;
constexpr int HTTP_PATCH   = 0x00000400;
constexpr int HTTP_TRACE   = 0x00000800;
constexpr int HTTP_CONNECT = 0x00001000;

// Code
constexpr int HTTP_CONTINUE            = 100;
constexpr int HTTP_SWITCHING_PROTOCOLS = 101;

constexpr int HTTP_OK              = 200;
constexpr int HTTP_CREATED         = 201;
constexpr int HTTP_ACCEPTED        = 202;
constexpr int HTTP_NO_CONTENT      = 204;
constexpr int HTTP_PARTIAL_CONTENT = 206;

constexpr int HTTP_MULITPLE_CHOICES  = 300;
constexpr int HTTP_MOVED_PERMANENTLY = 301;
constexpr int HTTP_FOUND             = 302;
constexpr int HTTP_SEE_OTHER         = 303;
constexpr int HTTP_NOT_MODIFIED      = 304;

constexpr int HTTP_BAD_REQUEST      = 400;
constexpr int HTTP_UNAUTHORIZED     = 401;
constexpr int HTTP_FORBINDDEN       = 403;
constexpr int HTTP_NOT_FOUND        = 404;
constexpr int HTTP_NOT_ALLOWED      = 405;
constexpr int HTTP_REQUEST_TIME_OUT = 408;

constexpr int HTTP_INTERNAL_SERVER_ERROR = 500;
constexpr int HTTP_NOT_IMPLEMENTED       = 501;
constexpr int HTTP_BAD_GATEWAY           = 502;
constexpr int HTTP_SERVICE_UNAVAILABLE   = 503;
constexpr int HTTP_GATEWAY_TIME_OUT      = 504;
constexpr int HTTP_VERSION_NOT_SUPPORT   = 505;
constexpr int HTTP_INSUFFICIENT_STORAGE  = 507;

constexpr char          cr   = '\r';
constexpr char          lf   = '\n';
inline std::string_view crlf = "\r\n";

inline std::string_view HTTP_v09 = "HTTP/0.9";
inline std::string_view HTTP_v10 = "HTTP/1.0";
inline std::string_view HTTP_v11 = "HTTP/1.1";
inline std::string_view HTTP_v20 = "HTTP/2.0";

inline std::string_view status_string(int code)
{
    switch (code) {
    case HTTP_OK:
        return "200 Ok";
    case HTTP_CREATED:
        return "201 Created";
    case HTTP_ACCEPTED:
        return "202 Accepted";
    case HTTP_NO_CONTENT:
        return "204 No Content";
    case HTTP_PARTIAL_CONTENT:
        return "206 Partial Content";

    case HTTP_MULITPLE_CHOICES:
        return "300 Multiple Choices";
    case HTTP_MOVED_PERMANENTLY:
        return "301 Moved Permanently";
    case HTTP_FOUND:
        return "302 Found";
        // TODO 3xx

    case HTTP_BAD_REQUEST:
        return "400 Bad Request";
    case HTTP_UNAUTHORIZED:
        return "401	Unauthorized";
    case HTTP_NOT_FOUND:
        return "404 Not Found";
    case HTTP_FORBINDDEN:
        return "403 Forbidden";
    case HTTP_NOT_ALLOWED:
        return "405 Method Not Allowed";
    case HTTP_REQUEST_TIME_OUT:
        return "408 Request Time Out";
        // TODO 4xx

    case HTTP_INTERNAL_SERVER_ERROR:
        return "500 Internal Server Error";
    case HTTP_NOT_IMPLEMENTED:
        return "501 Not Implemented";
    case HTTP_BAD_GATEWAY:
        return "502 Bad Gateway";
    case HTTP_VERSION_NOT_SUPPORT:
        return "505 Http Version Not Supported";
    // TODO 5x
    default:
        LHTTP_ASSERT(false);
    }
    return std::string_view{};
}
} // namespace lighthttp
