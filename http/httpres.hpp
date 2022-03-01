// http packet
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#include <string_view>

#include "buffer.hpp"
#include "httppar.hpp"
#include "mime.hpp"
//
//
//
//  httpd response
//
//
//
namespace pinkx {
namespace http {

using std::string_view;

#define SERVER_NAME "Dalek"
#define CRLF "\r\n"

static string_view err_page_tail =
    "<hr><center><span style='font-style: italic;'>" SERVER_NAME
    "</span></center>" CRLF "</body>" CRLF "</html>" CRLF;

static string_view err_301_page =
    "<html>" CRLF "<head><title>301 Moved Permanently</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>301 Moved Permanently</h1></center>" CRLF;

static string_view err_302_page =
    "<html>" CRLF "<head><title>302 Found</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF "<center><h1>302 Found</h1></center>" CRLF;

static string_view err_303_page =
    "<html>" CRLF "<head><title>303 See Other</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>303 See Other</h1></center>" CRLF;

static string_view err_307_page =
    "<html>" CRLF "<head><title>307 Temporary Redirect</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>307 Temporary Redirect</h1></center>" CRLF;

static string_view err_400_page =
    "<html>" CRLF "<head><title>400 Bad Request</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>400 Bad Request</h1></center>" CRLF;

static string_view err_401_page =
    "<html>" CRLF "<head><title>401 Authorization Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>401 Authorization Required</h1></center>" CRLF;

static string_view err_402_page =
    "<html>" CRLF "<head><title>402 Payment Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>402 Payment Required</h1></center>" CRLF;

static string_view err_403_page =
    "<html>" CRLF "<head><title>403 Forbidden</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>403 Forbidden</h1></center>" CRLF;

static string_view err_404_page =
    "<html>" CRLF "<head><title>404 Not Found</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>404 Not Found</h1></center>" CRLF;

static string_view err_405_page =
    "<html>" CRLF "<head><title>405 Not Allowed</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>405 Not Allowed</h1></center>" CRLF;

static string_view err_406_page =
    "<html>" CRLF "<head><title>406 Not Acceptable</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>406 Not Acceptable</h1></center>" CRLF;

static string_view err_407_page =
    "<html>" CRLF
    "<head><title>407 Proxy Authentication Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>407 Proxy Authentication Required</h1></center>" CRLF;

static string_view err_408_page =
    "<html>" CRLF "<head><title>408 Request Time-out</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>408 Request Time-out</h1></center>" CRLF;

static string_view err_409_page =
    "<html>" CRLF "<head><title>409 Conflict</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>409 Conflict</h1></center>" CRLF;

static string_view err_410_page =
    "<html>" CRLF "<head><title>410 Gone</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF "<center><h1>410 Gone</h1></center>" CRLF;

static string_view err_411_page =
    "<html>" CRLF "<head><title>411 Length Required</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>411 Length Required</h1></center>" CRLF;

static string_view err_412_page =
    "<html>" CRLF "<head><title>412 Precondition Failed</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>412 Precondition Failed</h1></center>" CRLF;

static string_view err_413_page =
    "<html>" CRLF
    "<head><title>413 Request Entity Too Large</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>413 Request Entity Too Large</h1></center>" CRLF;

static string_view err_414_page =
    "<html>" CRLF "<head><title>414 Request-URI Too Large</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>414 Request-URI Too Large</h1></center>" CRLF;

static string_view err_415_page =
    "<html>" CRLF "<head><title>415 Unsupported Media Type</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>415 Unsupported Media Type</h1></center>" CRLF;

static string_view err_416_page =
    "<html>" CRLF
    "<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF;

static string_view err_417_page =
    "<html>" CRLF "<head><title>417 Expectation Failed</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>417 Expectation Failed</h1></center>" CRLF;

static string_view err_500_page =
    "<html>" CRLF "<head><title>500 Internal Server Error</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>500 Internal Server Error</h1></center>" CRLF;

static string_view err_501_page =
    "<html>" CRLF "<head><title>501 Not Implemented</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>501 Not Implemented</h1></center>" CRLF;

static string_view err_502_page =
    "<html>" CRLF "<head><title>502 Bad Gateway</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>502 Bad Gateway</h1></center>" CRLF;

static string_view err_503_page =
    "<html>" CRLF
    "<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF;

static string_view err_504_page =
    "<html>" CRLF "<head><title>504 Gateway Time-out</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>504 Gateway Time-out</h1></center>" CRLF;

static string_view err_505_page =
    "<html>" CRLF
    "<head><title>505 HTTP Version Not Supported</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF;

static string_view err_507_page =
    "<html>" CRLF "<head><title>507 Insufficient Storage</title></head>" CRLF
    "<body bgcolor=\"white\">" CRLF
    "<center><h1>507 Insufficient Storage</h1></center>" CRLF;

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/
//
// 100 continue
// 200 ok: get head post trace
// The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No
// Content (or a 201 Created when the resource is uploaded for the first time).
// 201 create: put
// 202 delete put
// 404 not found
// 400 bad request
// 501 not implement
// 505 version not support


}  // namespace http
}  // namespace pinkx
#endif