#pragma once

#include "request.hpp"

namespace lighthttp
{
class streaming_parser
{
private:
    int         state_;
    int         done_;
    std::string parse_context_;

    enum
    {
        parsing_request = 0,
        parsing_heads,
        recving_body,
        parsing_done
    };

public:
    inline streaming_parser() :
        state_(0), done_(0){};
    ~streaming_parser() = default;

    streaming_parser(const streaming_parser &) = delete;
    streaming_parser &operator=(const streaming_parser &) = delete;

    int parse_request_line(char ch, request *r);
    int parse_head_line(char ch, request *r);
    int recv_body(char ch, request *r);

    inline bool request_is_done() const {
        return done_ > parsing_request;
    }

    inline bool heads_is_done() const {
        return done_ > parsing_heads;
    }

    inline bool is_done() const
    {
        return done_ == parsing_done;
    }

    inline void reset()
    {
        state_ = 0;
        done_  = 0;
        parse_context_.clear();
    }
};

inline int streaming_parser::parse_request_line(char ch, request *r)
{
    using namespace parser::util;
    using namespace method;
    using namespace version;
    using namespace code;
    using lighturl::imple::allow_in_path;

    switch (state_) {
    case rl_start:
        if (!allow_in_method(ch)) {
            return InvalidRequest;
        }
        r->method_.push_back(ch);
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
        r->method_.push_back(ch);
        break;

    case rl_url_start: {
        auto c = r->url_.parse(ch);
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

        auto c = r->url_.parse(ch);
        if (c == lighturl::Invalid) {
            return InvalidRequest;
        }
        break;
    }

    case rl_url_end:
        if (ch != 'H') {
            return InvalidRequest;
        }
        r->version_.push_back(ch);
        state_ = rl_http_h;
        break;

    case rl_http_h:
        if (ch != 'T') {
            return InvalidRequest;
        }
        r->version_.push_back(ch);
        state_ = rl_http_ht;
        break;

    case rl_http_ht:
        if (ch != 'T') {
            return InvalidRequest;
        }
        r->version_.push_back(ch);
        state_ = rl_http_htt;
        break;

    case rl_http_htt:
        if (ch != 'P') {
            return InvalidRequest;
        }
        r->version_.push_back(ch);
        state_ = rl_http_http;
        break;

    case rl_http_http:
        if (ch != '/') {
            return InvalidRequest;
        }
        r->version_.push_back('/');
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
        r->version_.push_back(ch);

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
        state_ = hl_start;
        done_  = parsing_heads;
        return RequestLineOk;

    default:
        assert(false);
    }

    return NotComplete;
}

inline int streaming_parser::parse_head_line(char ch, request *r)
{
    using namespace parser::util;
    using namespace method;
    using namespace version;
    using namespace code;
    using lighturl::imple::allow_in_path;

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
        r->heads_.emplace(parse_context_, std::string());
        r->heads_.at(parse_context_).push_back(ch);
        state_ = hl_value;
        break;

    case hl_value:
        if (ch == cr) {
            state_ = hl_almost_done;
            break;
        }
        if (ch != ' ' && !allow_in_path(ch)) {
            return InvalidHead;
        }
        r->heads_.at(parse_context_).push_back(ch);
        break;

    case hl_almost_done:
        if (ch != lf) {
            return InvalidHead;
        }
        state_ = hl_start;
        parse_context_.clear();
        return HeadLineOk;

    case hd_almost_done:
        if (ch != lf) {
            return InvalidHead;
        }
        state_ = bd_recving;
        parse_context_.clear();
        done_ = recving_body;
        r->check_chunked();
        r->check_keep_alive();
        r->check_body();
        if (r->content_length_ == 0 && !r->is_chunked())
        {
            done_ = parsing_done;
        }
        return HeadDone;

    default:
        return InvalidRequest;
    }
    return NotComplete;
}

inline int streaming_parser::recv_body(char ch, request* r)
{
    if (r->is_chunked()) {
        // TODO
        return InvalidRequest;
    }

    if (r->body_.size() < r->content_length_) {
        r->body_.push_back(ch);
    }

    if (r->body_.size() == r->content_length_) {
        done_ = parsing_done;
        return BodyDone;
    }

    return NotComplete;
}

} // namespace lighthttp