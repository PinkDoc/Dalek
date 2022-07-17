#pragma once

#include "common.hpp"
#include "url.hpp"

#include <map>
#include <vector>
#include <string_view>

#include <cassert>

namespace lighthttp
{
class head
{
private:
    std::string name_;
    std::string value_;

public:
    head()             = default;
    head(const head &) = default;
    head &operator=(const head &) = default;

    inline head(head &&h) :
        name_(std::move(h.name_)),
        value_(std::move(h.value_))
    {}

    inline head &operator=(head &&h)
    {
        name_  = std::move(h.name_);
        value_ = std::move(h.value_);
        return *this;
    }

    inline std::string &name_ref()
    {
        return name_;
    }

    inline std::string name()
    {
        return name_;
    }

    inline std::string &value_ref()
    {
        return value_;
    }

    inline std::string value()
    {
        return value_;
    }
};

class request
{
private:
    std::string                        method_;
    lighturl::url                      url_;
    std::string                        version_;
    std::map<std::string, std::string> heads_;
    int                                state_;
    int                                index_;
    std::size_t                        content_length_;
    std::string                        body_;

    bool        keep_alive_;
    std::string tranfer_encoding_;
    std::string parse_context_;

public:
    friend class streaming_parser;

    inline request() :
        state_(0),
        index_(0),
        content_length_(0),
        keep_alive_(false)
    {}

    request(const request &) = delete;
    request &operator=(const request &) = delete;
    inline request(request &&r) :
        method_(std::move(r.method_)),
        url_(std::move(r.url_)),
        version_(std::move(r.version_)),
        heads_(std::move(r.heads_)),
        state_(r.state_),
        index_(r.index_),
        content_length_(r.content_length_),
        body_(std::move(r.body_)),
        keep_alive_(r.keep_alive_),
        tranfer_encoding_(std::move(r.tranfer_encoding_)),
        parse_context_(std::move(r.parse_context_))
    {
        r.state_          = 0;
        r.index_          = 0;
        r.content_length_ = 0;
        r.keep_alive_     = false;
    }

    inline request &operator=(request &&r)
    {
        method_           = std::move(r.method_);
        url_              = std::move(r.url_);
        version_          = std::move(r.version_);
        heads_            = std::move(r.heads_);
        state_            = r.state_;
        index_            = r.index_;
        content_length_   = r.content_length_;
        body_             = std::move(r.body_);
        keep_alive_       = r.keep_alive_;
        tranfer_encoding_ = std::move(std::move(tranfer_encoding_));
        parse_context_    = std::move(r.parse_context_);
        r.state_          = 0;
        r.index_          = 0;
        r.content_length_ = 0;
        r.keep_alive_     = false;
        return *this;
    }

    inline std::string &method_ref()
    {
        return method_;
    }

    inline lighturl::url &url_ref()
    {
        return url_;
    }

    inline lighturl::url url() const
    {
        return url_;
    }

    inline std::string &version_ref()
    {
        return version_;
    }

    inline std::string version() const
    {
        return version_;
    }

    inline std::size_t content_length()
    {
        return content_length_;
    }

    inline std::string& body_ref() 
    {
        return body_;
    }

    inline bool is_chunked() const
    {
        return tranfer_encoding_ == std::string_view("chunked");
    }

    inline bool keep_alive() const
    {
        return keep_alive_;
    }

    // Return the ref of head's value
    inline const std::string &find_head_val_ref(const std::string &name) const
    {
        assert(heads_.find(name) != heads_.end());
        return const_cast<const std::string &>(heads_.at(name));
    }

    // Return the copy of head's value
    inline  std::string find_head_value(const std::string &name) const
    {
        if (heads_.find(name) == heads_.end()) {
            return std::string();
        }

        return heads_.at(name);
    }

    inline const std::map<std::string, std::string> &heads_ref()
    {
        return heads_;
    }

    inline void check_body()
    {
        auto iter = heads_.find("content-length");
        if (iter != heads_.end()) {
            auto len = std::atoi(iter->second.c_str());
            if (len > 0) {
                content_length_ = len;
                return;
            }
        }
        content_length_ = 0;
    }

    inline void check_keep_alive()
    {
        auto iter = heads_.find("connection");
        if (iter != heads_.end()) {
            keep_alive_ = iter->second == "keep-alive";
            return;
        }
        keep_alive_ = false;
    }

    inline void check_chunked()
    {
        auto iter = heads_.find("transfer-encoding");
        if (iter != heads_.end()) {
            tranfer_encoding_ = iter->second;
        }
    }

    // Start of buffer, and end of buffer
    int parse_request_line(const char *start, const char *end);
    int parse_request_line(const char *str, std::size_t l);
    int parse_request_line(const std::string &str);
    int parse_request_line(std::string_view str);
    int parse_head_line(const char *start, const char *end);
    int parse_head_line(const char *str, std::size_t l);
    int parse_head_line(const std::string &str);
    int parse_head_line(std::string_view str);
    int recv_body(const char *start, const char *end);
    int recv_body(const char *start, std::size_t l);

    inline void reset()
    {
        method_.clear();
        url_.reset();
        version_.clear();
        heads_.clear();
        state_          = 0;
        index_          = 0;
        content_length_ = 0;
        body_.clear();
        keep_alive_ = false;
        tranfer_encoding_.clear();
        parse_context_.clear();
    }
};

} // namespace lighthttp

#include "parser.hpp"
