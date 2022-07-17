#pragma once

#include "assert.hpp"
#include "common.hpp"
#include "error_page.hpp"

#include <map>
#include <functional>

namespace lighthttp
{
// Default version is http1.1
class response
{
private:
    std::map<std::string, std::string> heads_;
    std::string                        version_;
    int                                response_code_;
    std::size_t                        content_length_;
    std::string                        transfer_encoding_;

    std::string body_;

    inline void set_version(std::string_view v)
    {
        version_ = v;
    }

public:
    inline response() :
        response_code_(method::HTTP_UNKNOWN),
        content_length_(0),
        version_(version::HTTP_v11)
    {}

    response(const response &) = default;


    inline void set_version09()
    {
        set_version(version::HTTP_v09);
    }

    inline void set_version10()
    {
        set_version(version::HTTP_v10);
    }

    inline void set_version11()
    {
        set_version(version::HTTP_v11);
    }

    inline void set_version20()
    {
        set_version(version::HTTP_v20);
    }

    inline void set_status_code(int code)
    {
        response_code_ = code;
    }

    inline void handle_error(int code)
    {
        LHTTP_ASSERT(check_err_status(code));
        response_code_ = code;
        body_          = get_err_page(code);
    }

    inline void add_head(std::string k, std::string v)
    {
        heads_.emplace(std::move(k), std::move(v));
    }

    inline void set_content_length(std::size_t len)
    {
        heads_["content-length"] = std::move(std::to_string(len));
    }

    inline void set_encoding(std::string s)
    {
        transfer_encoding_       = s;
        heads_["content-coding"] = std::move(s);
    }

    inline void set_content_type(std::string s)
    {
        heads_["content-type"] = std::move(s);
    }

    inline void set_body(const std::string& body)
    {
        body_ = body;
    }

    inline  void set_body(std::string_view body)
    {
        body_ = body;
    }

    inline void set_body(const char* body)
    {
        LHTTP_ASSERT(body != nullptr);
        body_ = std::string(body);
    }

    inline void reset()
    {
        heads_.clear();
        version_ = version::HTTP_v11;
        response_code_ = method::HTTP_UNKNOWN;
        content_length_ = 0;
        transfer_encoding_.clear();
        body_.clear();
    }

    template <typename B>
    void append(B &buffer);
};

template <>
inline void response::append<std::string>(std::string& buffer)
{
    LHTTP_ASSERT(version_ != "");
    buffer.append(version_);
    buffer.push_back(' ');
    buffer.append(status_string(response_code_));
    buffer.append(crlf);
    
    if (body_.size() > 0)
    {
        add_head("content-length", std::to_string(body_.size()));
    }

    for (const auto& i : heads_)
    {
        buffer.append(i.first);
        buffer.append(" : ");
        buffer.append(i.second);
        buffer.append(crlf);
    }

    buffer.append(crlf);

    buffer.append(body_);
}

} // namespace lighthttp
