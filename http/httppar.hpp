// httpparser and request
#ifndef PINK_HTTP_PARSER_H
#define PINK_HTTP_PARSER_H

#include <sys/stat.h>

#include <string>
#include <utility>
#include <vector>

#include "mime.hpp"

/**
 * Implement a parser that works for non-blocking IO
 */
namespace pinkx {
namespace http {

class HttpHeader {
 private:
  typedef std::pair<std::string, std::string> HeadBuffer;

  HeadBuffer temp;

  std::vector<HeadBuffer> heads_;

 public:
  // vector value is a pair, and first is name, second is options
  auto& GetHeads() { return heads_; }

  std::string* GetValueByHead(std::string head) {
    for (auto& i : heads_) {
      if (i.first == head) {
        return &i.second;
      }
    }
    return nullptr;
  }

  // Only set to zero, and no delete
  void reset() { heads_.clear(); }
};

// Example: http://www.jobs.com/check.cgi?item=12731&color=blue&size=large
class HttpURI {
 public:
  HttpURI() : port(0){};
  // Only set to zero, and no delete
  void reset() {
    scheme.clear();
    host.clear();
    path.clear();
    query.clear();
    fragment.clear();
    port = 0;
  }

  std::string scheme;
  std::string host;
  std::string path;
  std::string query;
  std::string fragment;
  int port;
};

//  Http/1.1
enum method {
  HTTP_GET = 5000,
  HTTP_CONNECT,
  HTTP_DELETE,
  HTTP_HEAD,
  HTTP_OPTIONS,
  HTTP_PATCH,
  HTTP_POST,
  HTTP_PUT,
  HTTP_TRACE,
};

class HttpRequest {
 private:
  struct BodyBuffer {
    char* p;
    size_t len;

    BodyBuffer() : p(nullptr), len(0){};

    void reset() {
      p = nullptr;
      len = 0;
    }
  };

 public:
  HttpRequest()
      : version_(0),
        packet_(nullptr),
        len_(0),
        cgi(false),
        keep_alive_(false) {}

  rescode rescode_;
  HttpURI uri_;
  HttpHeader head_;
  method method_;
  BodyBuffer body_;
  int version_;
  // http packet_
  char* packet_;
  size_t len_;

  bool cgi;
  struct stat fileStat_;
  bool keep_alive_;

  // Only set to zero, and no delete
  void reset() {
    uri_.reset();
    head_.reset();
    body_.reset();

    version_ = 0;
    cgi = false;
    keep_alive_ = false;
  }

  std::string* GetValueByHead(std::string head) {
    return head_.GetValueByHead(head);
  }
};

//
//
//  http parser
//
//
#define str_cmp3(a, b) ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]))
#define str_cmp4(a, b) (str_cmp3(a, b) && (a[3] == b[3]))
#define str_cmp5(a, b) (str_cmp4(a, b) && (a[4] == b[4]))
#define str_cmp6(a, b) (str_cmp5(a, b) && (a[5] == b[5]))
#define str_cmp7(a, b) (str_cmp6(a, b) && (a[6] == b[6]))

#define CR "\r"
#define LF "\n"

//      http request packet:
//      [method] [uri] [version] \r\n
//      [head]\r\n
//      [head]\r\n
//      ...
//      \r\n
//      [resource]

enum parse_state {
  RL_BEGIN = 10000,
  RL_METHOD,
  RL_BEFORE_URI,
  RL_URI,
  RL_BEFORE_VERSION,
  RL_HTTP_H,
  RL_HTTP_HT,
  RL_HTTP_HTT,
  RL_HTTP_HTTP,
  RL_HTTP_HTTP_SLASH,
  RL_HTTP_VERSION_MAJOR,
  RL_HTTP_VERSION_DOT,
  RL_HTTP_VERSION_MINOR,
  RL_AFTER_VERSION,
  RL_ALMOST_DONE,
  RL_DONE,

  //  URI(http) : [scheme]://[host]:[port]/[resource]?[query]#[...]
  //  https://datatracker.ietf.org/doc/html/rfc2616/#section-9.3
  URI_BEGIN,
  URI_SCHEME,
  URI_SCHEME_COLORN,
  URI_SCHEME_SLASH,
  URI_SCEMEM_SLASH_SLASH,
  URI_HOST,
  URI_HOST_SLASH,
  URI_HOST_COLON,
  URI_PORT,
  URI_ABS_PATH_SLASH,
  URI_ABS_PATH,
  URI_QUERY,
  URI_FRAGMENT,

  // Head line

  HL_BEGIN,
  HL_IGNORE,
  HL_NAME,
  HL_NAME_COLON,
  HL_AFTER_NAME,
  HL_VALUE,
  HL_AFTER_VALUE,
  HL_ALMOST_DONE,
  HL_DONE,
  HL_BODY,
};

enum errcode {
  ERROR_REQUEST = -10,
  ERROR_URI,
  ERROR_HEAD,
  ERROR_EMPTY_REQUEST,

  REQUEST_OK = 1,
  URI_OK,
  // Only one line
  HEAD_OK,
  PARSE_OK,
  PARSE_AGAIN,
  PARSE_EMPTY_LINE,
  // All heads are ok
  PARSE_HEAD_OK,

};

enum checkcode { PARSE_REQUEST, PARSE_HEAD, PARSE_BODY };

// HttpParser
class HttpParser {
 private:
  checkcode check_code_;
  char* temp_;
  std::pair<std::string, std::string>* temp_head_;
  HttpRequest* request;
  parse_state parse_state_;
  parse_state uri_state_;
  // Save the parse state, offset is offset from packet's begin
  size_t offset;

 public:
  HttpParser()
      : parse_state_(RL_BEGIN),
        uri_state_(URI_BEGIN),
        offset(0),
        check_code_(PARSE_REQUEST){};
  HttpParser(HttpRequest& r)
      : parse_state_(RL_BEGIN),
        uri_state_(URI_BEGIN),
        offset(0),
        check_code_(PARSE_REQUEST),
        request(&r){};

  // Parse http packet
  errcode parse_HTTP();
  // Parse request line
  errcode parse_RL();

  // Parse URI
  errcode parse_URI(char ch);

  // Parse head line
  errcode parse_HL();

  void set_request(HttpRequest& r) { request = &r; }

  void reset() {
    parse_state_ = RL_BEGIN;
    uri_state_ = URI_BEGIN;
    check_code_ = PARSE_REQUEST;
    offset = 0;
  }

  size_t GetOffset() const { return offset; }

  parse_state ParseState() const { return parse_state_; }

  parse_state UriState() const { return uri_state_; }

  checkcode GetCheckCode() const { return check_code_; }
};

errcode HttpParser::parse_URI(char ch) {
  auto& uri = request->uri_;
  switch (uri_state_) {
    case URI_BEGIN:

      switch (ch) {
        case '/':
          uri_state_ = URI_ABS_PATH_SLASH;
          break;

        case 'A' ... 'Z':
        case 'a' ... 'z':
          uri.scheme.push_back(ch);
          uri_state_ = URI_SCHEME;
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_SCHEME:

      switch (ch) {
        case 'A' ... 'Z':
        case 'a' ... 'z':
          uri.scheme.push_back(ch);
          break;
        case ':':
          uri_state_ = URI_SCHEME_COLORN;
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_SCHEME_COLORN:

      switch (ch) {
        case '/':
          uri_state_ = URI_SCHEME_SLASH;
          break;

        default:
          return ERROR_URI;
      }

      break;

    case URI_SCHEME_SLASH:

      switch (ch) {
        case '/':
          uri_state_ = URI_SCEMEM_SLASH_SLASH;
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_SCEMEM_SLASH_SLASH:

      switch (ch) {
        case '0' ... '9':
        case 'A' ... 'Z':
        case 'a' ... 'z':
          uri_state_ = URI_HOST;
          uri.host.push_back(ch);
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_HOST:

      switch (ch) {
        case 'A' ... 'Z':
        case '1' ... '9':
        case 'a' ... 'z':
        case '-':
        case '.':
          uri.host.push_back(ch);
          break;
        case ':':
          // http://www.bing.com:80/index.html
          // http://123.56.1.12:1989/index.html
          uri_state_ = URI_PORT;
          break;
        case '/':
          // http://www.bing.com/index.html
          uri_state_ = URI_ABS_PATH_SLASH;
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_PORT:

      switch (ch) {
        case ' ':
          uri_state_ = URI_BEGIN;
          break;
        case '0' ... '9':
          uri.port = uri.port * 10 + ch - '0';
          break;
        case '/':
          uri_state_ = URI_ABS_PATH_SLASH;
          break;
        default:
          return ERROR_URI;
      }

      break;

// Abs path and configure
#define ALLOW_ABS_PATH                                                    \
  'A' ... 'Z' : case '/' : case 'a' ... 'z' : case '0' ... '9' : case '.' \
      : case '-' : case '_' : case '!' : case '~' : case '*' : case '\''  \
      : case '(' : case ')' : case '%' : case ':' : case '@' : case '&'   \
      : case '=' : case '+' : case '$' : case ','

#define ALLOW_IN_QUERY '?' : case ALLOW_ABS_PATH

    case URI_ABS_PATH_SLASH:

      switch (ch) {
        case ALLOW_ABS_PATH:
          uri.path.push_back(ch);
          uri_state_ = URI_ABS_PATH;
          break;
        case '?':
          uri_state_ = URI_QUERY;
          break;
        case '#':
          uri_state_ = URI_FRAGMENT;
          break;
        default:
          break;
      }

      break;

    case URI_ABS_PATH:

      switch (ch) {
        case ALLOW_ABS_PATH:
          uri.path.push_back(ch);
          break;
        case '?':
          uri_state_ = URI_QUERY;
          break;
        case '#':
          uri_state_ = URI_FRAGMENT;
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_QUERY:

      switch (ch) {
        case '#':
          uri_state_ = URI_FRAGMENT;
          break;
        case ALLOW_IN_QUERY:
          uri.query.push_back(ch);
          break;
        default:
          return ERROR_URI;
      }

      break;

    case URI_FRAGMENT:

      switch (ch) {
        default:
          uri.fragment.push_back(ch);
          break;
      }

      break;

    default:
      return ERROR_URI;
  }
  return URI_OK;
}

errcode HttpParser::parse_RL() {
  char* end = request->packet_ + request->len_;
  char* begin = request->packet_ + offset;
  for (; begin < end; ++begin) {
    char ch = *begin;
    switch (parse_state_) {
      case RL_BEGIN: {
        switch (ch) {
          case 'A' ... 'Z':
            temp_ = begin;
            parse_state_ = RL_METHOD;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_METHOD: {
        switch (ch) {
          case 'A' ... 'Z':
            break;
          case ' ': {
            char* tmp = temp_;
            size_t mlen = begin - tmp;
            switch (mlen) {
              case 3:
                if (str_cmp3(tmp, "GET")) {
                  request->method_ = HTTP_GET;
                } else if (str_cmp3(tmp, "PUT")) {
                  request->method_ = HTTP_PUT;
                } else
                  return ERROR_REQUEST;
                break;

              case 4:
                if (str_cmp4(tmp, "POST")) {
                  request->method_ = HTTP_POST;
                } else if (str_cmp4(tmp, "HEAD")) {
                  request->method_ = HTTP_HEAD;
                } else {
                  return ERROR_REQUEST;
                }
                break;

              case 5:
                if (str_cmp5(tmp, "PATCH")) {
                  request->method_ = HTTP_PATCH;
                } else if (str_cmp5(tmp, "TRACE")) {
                  request->method_ = HTTP_TRACE;
                } else {
                  return ERROR_REQUEST;
                }
                break;

              case 6:
                if (str_cmp6(tmp, "DELETE")) {
                  request->method_ = HTTP_DELETE;
                } else {
                  return ERROR_REQUEST;
                }
                break;

              case 7:
                if (str_cmp7(tmp, "CONNECT")) {
                  request->method_ = HTTP_CONNECT;
                } else if (str_cmp7(tmp, "OPTIONS")) {
                  request->method_ = HTTP_OPTIONS;
                } else {
                  return ERROR_REQUEST;
                }
                break;

              default:
                return ERROR_REQUEST;
            }
            parse_state_ = RL_BEFORE_URI;
          } break;
          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_BEFORE_URI: {
        switch (ch) {
          case ' ':
            break;
          case 'a' ... 'z':
          case 'A' ... 'Z':
          case '/':
            uri_state_ = URI_BEGIN;
            parse_state_ = RL_URI;
            if (parse_URI(ch) != URI_OK) {
              return ERROR_URI;
            }
            break;
          default:
            return ERROR_URI;
        }
      } break;

      case RL_URI: {
        switch (ch) {
          case ' ':
            parse_state_ = RL_BEFORE_VERSION;
            break;

          default:
            if (parse_URI(ch) != URI_OK) {
              return ERROR_URI;
            }
            break;
        }
      } break;

      case RL_BEFORE_VERSION: {
        switch (ch) {
          case ' ':
            break;
          case 'H':
            parse_state_ = RL_HTTP_H;
            break;
          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_H: {
        switch (ch) {
          case 'T':
            parse_state_ = RL_HTTP_HT;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_HT: {
        switch (ch) {
          case 'T':
            parse_state_ = RL_HTTP_HTT;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_HTT: {
        switch (ch) {
          case 'P':
            parse_state_ = RL_HTTP_HTTP;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_HTTP: {
        switch (ch) {
          case '/':
            parse_state_ = RL_HTTP_HTTP_SLASH;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_HTTP_SLASH: {
        switch (ch) {
          case '0' ... '9':
            request->version_ = request->version_ * 10 + ch - '0';
            parse_state_ = RL_HTTP_VERSION_MAJOR;
            break;
          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_VERSION_MAJOR: {
        switch (ch) {
          case '0' ... '9':
            request->version_ = request->version_ * 10 + ch - '0';
            break;
          case '.':
            parse_state_ = RL_HTTP_VERSION_DOT;
            break;
          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_VERSION_DOT: {
        switch (ch) {
          case '0' ... '9':
            request->version_ = request->version_ * 10 + ch - '0';
            parse_state_ = RL_HTTP_VERSION_MINOR;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_HTTP_VERSION_MINOR: {
        switch (ch) {
          case '0' ... '9':
            request->version_ = request->version_ * 10 + ch - '0';
            break;
          case ' ':
            parse_state_ = RL_AFTER_VERSION;
            break;
          case '\r':
            parse_state_ = RL_ALMOST_DONE;
            break;
          case '\n':
            parse_state_ = RL_DONE;
            break;
          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_AFTER_VERSION: {
        switch (ch) {
          case '\r':
            parse_state_ = RL_ALMOST_DONE;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_ALMOST_DONE: {
        switch (ch) {
          case '\n':
            parse_state_ = RL_DONE;
            break;

          default:
            return ERROR_REQUEST;
        }
      } break;

      case RL_DONE: {
        offset = begin - request->packet_;
        check_code_ = PARSE_HEAD;
        parse_state_ = HL_BEGIN;
        return REQUEST_OK;
      } break;
      default:
        return ERROR_REQUEST;
    }
  }
  // Update offset
  offset = begin - request->packet_;
  return PARSE_AGAIN;
}

errcode HttpParser::parse_HL() {
  char* begin = request->packet_ + offset;
  char* end = request->packet_ + request->len_;
  for (; begin < end; ++begin) {
    char ch = *begin;
    switch (parse_state_) {
      case HL_BEGIN:

        switch (ch) {
          case '\r':
            temp_head_ = nullptr;
            parse_state_ = HL_ALMOST_DONE;
            break;

          case '\n':
            temp_head_ = nullptr;
            goto done;

          default: {
            auto& heads = request->head_.GetHeads();
            size_t p = heads.size();
            heads.emplace_back("", "");
            temp_head_ = &heads[p];
            if (ch == '-') ch = ch - '-' + '_';
            temp_head_->first.push_back(ch);
            parse_state_ = HL_NAME;
          }
        }

        break;

      case HL_NAME:
        switch (ch) {
          case '\r':
            parse_state_ = HL_ALMOST_DONE;
            break;
          case '\n':
            goto done;
          case ' ':
            break;
          case ':':
            parse_state_ = HL_NAME_COLON;
            break;
          default:
            if (ch == '-')
              ch = ch - '-' + '_';
            else if (ch >= 'A' && ch <= 'Z')
              ch = ch - 'A' + 'a';
            temp_head_->first.push_back(ch);
            break;
        }

        break;

      case HL_NAME_COLON:

        switch (ch) {
          case ' ':
            parse_state_ = HL_AFTER_NAME;
            break;

          default:
            temp_head_->second.push_back(ch);
            parse_state_ = HL_VALUE;
        }

        break;

      case HL_AFTER_NAME:

        switch (ch) {
          case ' ':
            break;

          default:
            temp_head_->second.push_back(ch);
            parse_state_ = HL_VALUE;
        }

        break;

      case HL_VALUE:

        switch (ch) {
          case '\r':
            parse_state_ = HL_ALMOST_DONE;
            break;
          case '\n':
            goto done;
          default:
            temp_head_->second.push_back(ch);
        }

        break;

      case HL_ALMOST_DONE:
        switch (ch) {
          case '\n':
            goto done;
            break;

          default:
            return ERROR_HEAD;
        }

        break;
    }
  }
  offset = begin - request->packet_;
  return PARSE_AGAIN;
done:
  offset = begin - request->packet_ + 1;
  parse_state_ = HL_BEGIN;
  return temp_head_ == nullptr ? PARSE_EMPTY_LINE : PARSE_HEAD_OK;
}

errcode HttpParser::parse_HTTP() {
  if (request == nullptr) {
    return ERROR_EMPTY_REQUEST;
  }

  errcode code = PARSE_OK;
  while (true) {
    switch (check_code_) {
      case PARSE_REQUEST:
        code = parse_RL();
        if (code != REQUEST_OK) return code;
        break;
      case PARSE_HEAD:
        // Parse heads
        for (;;) {
          auto c = parse_HL();
          if (c == PARSE_AGAIN) {
            code = c;
            break;
          } else if (c == PARSE_HEAD_OK) {
            // Do nothing
          } else if (c == PARSE_EMPTY_LINE) {
            check_code_ = PARSE_BODY;
            code = c;
            break;
          } else {
            // Error
            code = c;
            break;
          }
        }
        // After update heads
        if (code != PARSE_EMPTY_LINE) {
          return code;
        }

        break;

      case PARSE_BODY:
        return PARSE_EMPTY_LINE;
        break;

      default:
        return code;
    }
  }
  return code;
}

}  // namespace http
}  // namespace pinkx
#endif