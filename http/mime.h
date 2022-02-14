// mime
#ifndef PINKX_MIME_H
#define PINKX_MIME_H

#include <string_view>
#include <unordered_map>

namespace pinkx {

using std::string_view;
typedef std::unordered_map<string_view, string_view> StringMap;

inline StringMap mimeMap = {
    {"default", "text/plain"},   {".txt", "text/plain"},
    {".html", "text/html"},      {".css", "text/css"},
    {".jsp", "text/javascript"}, {".png", "image/png"},
    {".gif", "image/gif"},       {".jpeg", "image/jpeg"},
    {".bmp", "image/bmp"},       {".webp", "image/webp"},
    {".x-icon", "image/x-icon"}, {".midi", "audio/midi"},
    {".mpeg", "audio/mpeg"},     {".webm", "audio/webm"},
    {".wav", "audio/wav"},       {".pdf", "application/pdf"},
    {".webm", "video/webm"},     {".mp4", "video/mp4"},
    {".ogg", "video/ogg"},
};

enum rescode {
  OK,
  CREATE,
  ACCEPT,
  NOT_FOUND,
  BAD_REQUEST,
  NOT_IMPLEMENT,
  FORBIDDEN,
  INTERNAL_SERVER_ERROR,  // cgi not support
  VERSION_NOT_SUPPORT
};

inline std::unordered_map<rescode, std::string_view> resMap = {
    {OK, "200 OK "},
    {CREATE, "201 Created "},
    {ACCEPT, "202 Accepted "},
    {NOT_FOUND, "404 Not Found "},
    {FORBIDDEN, "403 Forbidden "},
    {BAD_REQUEST, "400 Bad Request "},
    {INTERNAL_SERVER_ERROR, "500 Internal Server Error"},
    {NOT_IMPLEMENT, "501 Not Implement "},
    {VERSION_NOT_SUPPORT, "505 Version Not Support "},

};
}  // namespace pinkx

#endif