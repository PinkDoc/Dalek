#pragma once

#include "assert.hpp"
#include <string_view>

namespace lighthttp
{
enum parse_status
{   
    InvalidRequest = -2,
    InvalidHead,
    NotComplete,
    RequestLineOk, // Request parse ok
    HeadLineOk,    // One head
    HeadDone,
    BodyDone
};

namespace method 
{
// Method
inline constexpr int HTTP_UNKNOWN = 0X00000001;
inline constexpr int HTTP_GET     = 0x00000002;
inline constexpr int HTTP_HEAD    = 0x00000004;
inline constexpr int HTTP_POST    = 0x00000008;
inline constexpr int HTTP_PUT     = 0x00000010;
inline constexpr int HTTP_DELETE  = 0x00000020;
inline constexpr int HTTP_OPTIONS = 0x00000200;
inline constexpr int HTTP_PATCH   = 0x00000400;
inline constexpr int HTTP_TRACE   = 0x00000800;
inline constexpr int HTTP_CONNECT = 0x00001000;
} // namespace method

namespace code 
{
// Code
inline constexpr int HTTP_CONTINUE            = 100;
inline constexpr int HTTP_SWITCHING_PROTOCOLS = 101;

inline constexpr int HTTP_OK              = 200;
inline constexpr int HTTP_CREATED         = 201;
inline constexpr int HTTP_ACCEPTED        = 202;
inline constexpr int HTTP_NO_CONTENT      = 204;
inline constexpr int HTTP_PARTIAL_CONTENT = 206;

inline constexpr int HTTP_MULITPLE_CHOICES  = 300;
inline constexpr int HTTP_MOVED_PERMANENTLY = 301;
inline constexpr int HTTP_FOUND             = 302;
inline constexpr int HTTP_SEE_OTHER         = 303;
inline constexpr int HTTP_NOT_MODIFIED      = 304;

inline constexpr int HTTP_BAD_REQUEST      = 400;
inline constexpr int HTTP_UNAUTHORIZED     = 401;
inline constexpr int HTTP_FORBINDDEN       = 403;
inline constexpr int HTTP_NOT_FOUND        = 404;
inline constexpr int HTTP_NOT_ALLOWED      = 405;
inline constexpr int HTTP_REQUEST_TIME_OUT = 408;

inline constexpr int HTTP_INTERNAL_SERVER_ERROR = 500;
inline constexpr int HTTP_NOT_IMPLEMENTED       = 501;
inline constexpr int HTTP_BAD_GATEWAY           = 502;
inline constexpr int HTTP_SERVICE_UNAVAILABLE   = 503;
inline constexpr int HTTP_GATEWAY_TIME_OUT      = 504;
inline constexpr int HTTP_VERSION_NOT_SUPPORT   = 505;
inline constexpr int HTTP_INSUFFICIENT_STORAGE  = 507;
}

inline constexpr char          cr   = '\r';
inline constexpr char          lf   = '\n';
inline std::string_view crlf = "\r\n";

namespace version 
{
inline std::string_view HTTP_v09 = "HTTP/0.9";
inline std::string_view HTTP_v10 = "HTTP/1.0";
inline std::string_view HTTP_v11 = "HTTP/1.1";
inline std::string_view HTTP_v20 = "HTTP/2.0";
}

inline std::string_view status_string(int c)
{
    using namespace code;
    switch (c) {
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
