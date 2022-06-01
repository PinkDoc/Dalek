#pragma once

#include "common.hpp"

namespace lighthttp
{
inline std::string_view err_page_tail =
    "<hr><center><span style='font-style: italic;'>"
    "</span></center>"
    "</body>"
    "</html>";

inline std::string_view err_301_page =
    "<html>"
    "<head><title>301 Moved Permanently</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>301 Moved Permanently</h1></center>";

inline std::string_view err_302_page =
    "<html>"
    "<head><title>302 Found</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>302 Found</h1></center>";

inline std::string_view err_303_page =
    "<html>"
    "<head><title>303 See Other</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>303 See Other</h1></center>";

inline std::string_view err_307_page =
    "<html>"
    "<head><title>307 Temporary Redirect</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>307 Temporary Redirect</h1></center>";

inline std::string_view err_400_page =
    "<html>"
    "<head><title>400 Bad Request</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>400 Bad Request</h1></center>";

inline std::string_view err_401_page =
    "<html>"
    "<head><title>401 Authorization Required</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>401 Authorization Required</h1></center>";

inline std::string_view err_402_page =
    "<html>"
    "<head><title>402 Payment Required</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>402 Payment Required</h1></center>";

inline std::string_view err_403_page =
    "<html>"
    "<head><title>403 Forbidden</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>403 Forbidden</h1></center>";

inline std::string_view err_404_page =
    "<html>"
    "<head><title>404 Not Found</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>404 Not Found</h1></center>";

inline std::string_view err_405_page =
    "<html>"
    "<head><title>405 Not Allowed</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>405 Not Allowed</h1></center>";

inline std::string_view err_406_page =
    "<html>"
    "<head><title>406 Not Acceptable</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>406 Not Acceptable</h1></center>";

inline std::string_view err_407_page =
    "<html>"
    "<head><title>407 Proxy Authentication Required</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>407 Proxy Authentication Required</h1></center>";

inline std::string_view err_408_page =
    "<html>"
    "<head><title>408 Request Time-out</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>408 Request Time-out</h1></center>";

inline std::string_view err_409_page =
    "<html>"
    "<head><title>409 Conflict</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>409 Conflict</h1></center>";

inline std::string_view err_410_page =
    "<html>"
    "<head><title>410 Gone</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>410 Gone</h1></center>";

inline std::string_view err_411_page =
    "<html>"
    "<head><title>411 Length Required</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>411 Length Required</h1></center>";

inline std::string_view err_412_page =
    "<html>"
    "<head><title>412 Precondition Failed</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>412 Precondition Failed</h1></center>";

inline std::string_view err_413_page =
    "<html>"
    "<head><title>413 Request Entity Too Large</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>413 Request Entity Too Large</h1></center>";

inline std::string_view err_414_page =
    "<html>"
    "<head><title>414 Request-URI Too Large</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>414 Request-URI Too Large</h1></center>";

inline std::string_view err_415_page =
    "<html>"
    "<head><title>415 Unsupported Media Type</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>415 Unsupported Media Type</h1></center>";

inline std::string_view err_416_page =
    "<html>"
    "<head><title>416 Requested Range Not Satisfiable</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>416 Requested Range Not Satisfiable</h1></center>";

inline std::string_view err_417_page =
    "<html>"
    "<head><title>417 Expectation Failed</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>417 Expectation Failed</h1></center>";

inline std::string_view err_500_page =
    "<html>"
    "<head><title>500 Internal Server Error</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>500 Internal Server Error</h1></center>";

inline std::string_view err_501_page =
    "<html>"
    "<head><title>501 Not Implemented</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>501 Not Implemented</h1></center>";

inline std::string_view err_502_page =
    "<html>"
    "<head><title>502 Bad Gateway</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>502 Bad Gateway</h1></center>";

inline std::string_view err_503_page =
    "<html>"
    "<head><title>503 Service Temporarily Unavailable</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>503 Service Temporarily Unavailable</h1></center>";

inline std::string_view err_504_page =
    "<html>"
    "<head><title>504 Gateway Time-out</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>504 Gateway Time-out</h1></center>";

inline std::string_view err_505_page =
    "<html>"
    "<head><title>505 HTTP Version Not Supported</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>505 HTTP Version Not Supported</h1></center>";

inline std::string_view err_507_page =
    "<html>"
    "<head><title>507 Insufficient Storage</title></head>"
    "<body bgcolor=\"white\">"
    "<center><h1>507 Insufficient Storage</h1></center>";

} // namespace lighthttp
