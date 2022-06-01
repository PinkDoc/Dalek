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

    head(head &&h) :
        name_(std::move(h.name_)),
        value_(std::move(h.value_))
    {}

    head &operator=(head &&h)
    {
        name_  = std::move(h.name_);
        value_ = std::move(h.value_);
        return *this;
    }

    std::string &name_ref()
    {
        return name_;
    }

    std::string name()
    {
        return name_;
    }

    std::string &value_ref()
    {
        return value_;
    }

    std::string value()
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

    std::string parse_context_;

public:
    request() :
        state_(0)
    {}

    request(const request &) = delete;
    request &operator=(const request &) = delete;
    request(request &&r) :
        method_(std::move(r.method_)),
        url_(std::move(r.url_)),
        version_(std::move(r.version_)),
        heads_(std::move(r.heads_)),
        state_(r.state_),
        parse_context_(std::move(r.parse_context_))
    {
        r.state_ = 0;
    }

    request &operator=(request &&r)
    {
        method_        = std::move(r.method_);
        url_           = std::move(r.url_);
        version_       = std::move(r.version_);
        heads_         = std::move(r.heads_);
        state_         = r.state_;
        parse_context_ = std::move(r.parse_context_);
        r.state_       = 0;
        return *this;
    }

    std::string &method_ref()
    {
        return method_;
    }

    lighturl::url &url_ref()
    {
        return url_;
    }

    lighturl::url url()
    {
        return url_;
    }

    std::string &version_ref()
    {
        return version_;
    }

    std::string version()
    {
        return version_;
    }

    // Return the ref of head's value
    const std::string &find_head_val_ref(const std::string &name) const
    {
        assert(heads_.find(name) == heads_.end());
        return const_cast<const std::string &>(heads_.at(name));
    }

    // Return the copy of head's value
    std::string find_head_value(const std::string &name) const
    {
        if (heads_.find(name) == heads_.end()) {
            return std::string();
        }

        return heads_.at(name);
    }

    int parse_request_line(const char *start, const char *end);
    int parse_request_line(const char *str, std::size_t l);
    int parse_request_line(const std::string &str);
    int parse_request_line(std::string_view str);
    int parse_head_line(const char *start, const char *end);
    int parse_head_line(const char *str, std::size_t l);
    int parse_head_line(const std::string &str);
    int parse_head_line(std::string_view str);

    void reset()
    {
        method_.clear();
        url_.reset();
        version_.clear();
        heads_.clear();
        state_ = 0;
        parse_context_.clear();
    }
};

} // namespace lighthttp

#include "parser.hpp"
