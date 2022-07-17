#pragma once

#include "request.hpp"

namespace lighthttp
{
namespace parser
{
namespace util
{
inline bool cmp3(char *m, char *n)
{
    return m[0] == n[0] && m[1] == n[1] && m[2] == n[2];
}

inline bool cmp4(char *m, char *n)
{
    return cmp3(m, n) && m[3] == n[3];
}

inline bool cmp5(char *m, char *n)
{
    return cmp4(m, n) && m[4] == n[4];
}

inline bool cmp6(char *m, char *n)
{
    return cmp5(m, n) && m[5] == n[5];
}

inline bool cmp7(char *m, char *n)
{
    return cmp6(m, n) && m[6] == n[6];
}

inline bool cmp8(char *m, char *n)
{
    return cmp7(m, n) && m[7] == n[7];
}

inline bool cmp9(char *m, char *n)
{
    return cmp8(m, n) && m[8] == n[8];
}

enum
{
    rl_start = 0,
    rl_method,
    rl_url_start,
    rl_url,
    rl_url_end,
    rl_http_start,
    rl_http_h,
    rl_http_ht,
    rl_http_htt,
    rl_http_http,
    rl_http_version,
    rl_after_version,
    rl_almost_done,
    hl_start,
    hl_name,
    hl_after_name,
    hl_before_value,
    hl_value,
    hl_almost_done,
    hd_almost_done,
    bd_recving,          // recv body
    req_done
};

inline bool allow_in_method(char ch)
{
    return (ch >= 'A' && ch <= 'Z');
}

inline char to_lower(char ch)
{
    if (ch >= 'A' && ch <= 'Z') return (ch - 'A' + 'a');
    return ch;
}

} // namespace parser
} // namespace imple

inline int request::parse_request_line(const char *start, const char *end)
{
    using namespace parser::util;
    using namespace method;
    using namespace version;
    using namespace code;
    
    auto i = start + index_;
    for (; i < end; ++i) {
        char ch = *i;
        switch (state_) {
        case rl_start:
            if (!allow_in_method(ch)) {
                return InvalidRequest;
            }
            method_.push_back(ch);
            state_ = rl_method;
            break;

        case rl_method:
            if (ch == ' ') {
                state_ = rl_url_start;
                break;
            }

            if (!allow_in_method(ch)) {
                return InvalidRequest;
            }
            method_.push_back(ch);
            break;

        case rl_url_start: {
            auto c = url_.parse(ch);
            if (c == lighturl::Invalid) {
                return InvalidRequest;
            }
            state_ = rl_url;
            break;
        }

        case rl_url: {
            if (ch == ' ') {
                state_ = rl_url_end;
                break;
            }

            auto c = url_.parse(ch);
            if (c == lighturl::Invalid) {
                return InvalidRequest;
            }
            break;
        }

        case rl_url_end:
            if (ch != 'H') {
                return InvalidRequest;
            }
            version_.push_back(ch);
            state_ = rl_http_h;
            break;

        case rl_http_h:
            if (ch != 'T') {
                return InvalidRequest;
            }
            version_.push_back(ch);
            state_ = rl_http_ht;
            break;

        case rl_http_ht:
            if (ch != 'T') {
                return InvalidRequest;
            }
            version_.push_back(ch);
            state_ = rl_http_htt;
            break;

        case rl_http_htt:
            if (ch != 'P') {
                return InvalidRequest;
            }
            version_.push_back(ch);
            state_ = rl_http_http;
            break;

        case rl_http_http:
            if (ch != '/') {
                return InvalidRequest;
            }
            version_.push_back('/');
            state_ = rl_http_version;
            break;

        case rl_http_version:
            if (ch == ' ') {
                state_ = rl_after_version;
                break;
            }

            if (ch == cr) {
                state_ = rl_almost_done;
                break;
            }

            if (!((ch >= '0' && ch <= '9') || ch == '.')) {
                return InvalidRequest;
            }
            version_.push_back(ch);

            break;

        case rl_after_version:
            if (ch == ' ') {
                break;
            }
            if (ch == cr) {
                state_ = rl_almost_done;
                break;
            }
            return InvalidRequest;
            break;

        case rl_almost_done:
            if (ch != lf) {
                return InvalidRequest;
            }
            goto rl_done;

        default:
            assert(false);
        }
    }
    index_ = i - start;
    return NotComplete;
rl_done:
    index_ = i + 1 - start;
    state_ = hl_start;
    return RequestLineOk;
}

inline int request::parse_request_line(const char *str, std::size_t l)
{
    return parse_request_line(str, str + l);
}

inline int request::parse_request_line(const std::string &str)
{
    return parse_request_line(str.data(), str.data() + str.size());
}

inline int request::parse_request_line(std::string_view str)
{
    return parse_request_line(str.data(), str.data() + str.size());
}

// All head name turn to lowercase
inline int request::parse_head_line(const char *start, const char *end)
{
    using namespace parser::util;
    using namespace method;
    using namespace version;
    using namespace code;
    using lighturl::imple::allow_in_path;

    auto i = start + index_;
    for ( ; i < end; ++i) {
        char ch = *i;
        switch (state_) {
        case hl_start:
            if (ch == cr) {
                state_ = hd_almost_done;
                break;
            }
            if (!allow_in_path(ch)) {
                return InvalidHead;
            }
            parse_context_.clear();
            parse_context_.push_back(to_lower(ch));
            state_ = hl_name;
            break;

        case hl_name:
            if (ch == ' ') {
                state_ = hl_after_name;
                break;
            }
            if (ch == ':') {
                state_ = hl_before_value;
                break;
            }
            if (!allow_in_path(ch)) {
                return InvalidHead;
            }

            parse_context_.push_back(to_lower(ch));
            break;

        case hl_after_name:
            if (ch == ' ') {
                break;
            }
            if (ch == ':') {
                state_ = hl_before_value;
                break;
            }
            return InvalidHead;

        case hl_before_value:
            if (ch == ' ') {
                break;
            }
            if (!allow_in_path(ch)) {
                return InvalidHead;
            }
            heads_.emplace(parse_context_, std::string());
            heads_.at(parse_context_).push_back(ch);
            state_ = hl_value;
            break;

        case hl_value:
            if (ch == cr) {
                state_ = hl_almost_done;
                break;
            }
            if (ch != ' '&& !allow_in_path(ch)) {
                return InvalidHead;
            }
            heads_.at(parse_context_).push_back(ch);
            break;

        case hl_almost_done:
            if (ch != lf) {
                return InvalidHead;
            }
            goto hl_done;

        case hd_almost_done:
            if (ch != lf) {
                return InvalidHead;
            }
            goto hd_done;
        default:
            return InvalidRequest;
        }
    }
    index_ = i - start;
    return NotComplete;
hl_done:
    index_ = i + 1 - start;
    state_ = hl_start;
    return HeadLineOk;
hd_done:
    index_ = i + 1 - start;
    check_body();
    if (content_length_ == 0)
    {
        state_ = req_done;
    }
    else 
    {
        state_ = bd_recving;
    }
    return HeadDone;
}

inline int request::parse_head_line(const char *str, std::size_t l)
{
    return parse_head_line(str, str + l);
}

inline int request::parse_head_line(const std::string &str)
{
    return parse_head_line(str.data(), str.data() + str.size());
}

inline int request::parse_head_line(std::string_view str)
{
    return parse_head_line(str.data(), str.data() + str.size());
}

inline int request::recv_body(const char* start, const char* end)
{
    auto i = start + index_;

    for (; body_.size() < content_length_ && i < end; ++i)
    {
        char ch = *i;
        body_.push_back(ch);
    }

    assert(body_.size() <= content_length_);

    index_ = i - start;

    return (body_.size() == content_length_ ? parse_status::BodyDone : parse_status::NotComplete);
}

inline int request::recv_body(const char* start, std::size_t len)
{
    return recv_body(start, start+len);
}

} // namespace lighthttp
