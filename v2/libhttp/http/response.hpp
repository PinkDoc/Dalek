#pragma once

#include "assert.hpp"
#include "common.hpp"
#include "error_page.hpp"

#include <map>
#include <functional>

namespace lighthttp
{
class response
{
private:
    std::map<std::string, std::string> heads_;
    std::string                        version_;
    int                                response_code_;
    std::size_t                        content_length_;
    std::string                        transfer_encoding;

    void generate_body(std::string &s);
    void set_version(std::string_view v)
    {
        version_ = v;
    }

public:
    response() :
        response_code_(HTTP_UNKNOWN),
        content_length_(0)
    {}

    void set_version09()
    {
        set_version(HTTP_v09);
    }

    void set_version10()
    {
        set_version(HTTP_v10);
    }

    void set_version11()
    {
        set_version(HTTP_v11);
    }

    void set_version20()
    {
        set_version(HTTP_v20);
    }

    void set_status_code(int code)
    {
        response_code_ = code;
    }

    void add_head(std::string k, std::string v)
    {
        heads_.emplace(std::move(k), std::move(v));
    }

    void set_content_length(std::size_t len)
    {
        heads_["content-length"] = std::move(std::to_string(len));
    }

    void set_encoding(std::string s)
    {
        heads_["content-coding"] = std::move(s);
    }

    void set_content_type(std::string s)
    {
        heads_["content-type"] = std::move(s);
    }

    // No body :)
    std::string generate_str()
    {
        std::string res;

        LHTTP_ASSERT(version_ == HTTP_v09
                     || version_ == HTTP_v10
                     || version_ == HTTP_v11
                     || version_ == HTTP_v20);

        res.append(version_);
        res.push_back(' ');
        res.append(status_string(response_code_));
        res.append(crlf);

        if (response_code_ >= 400) {
            heads_["content-type"] = "text/html";
        }

        for (auto &i : heads_) {
            res.append(std::move(i.first));
            res.append(" : ");
            res.append(std::move(i.second));
            res.append(crlf);
        }

        if (response_code_ >= 400) {
            generate_body(res);
        }

        return res;
    }

};

inline void response::generate_body(std::string &s)
{
    switch (response_code_) {
    case HTTP_MOVED_PERMANENTLY:
        s.append(err_301_page);
        s.append(err_page_tail);
        break;
    case HTTP_FOUND:
        s.append(err_302_page);
        s.append(err_page_tail);
        break;
    case HTTP_SEE_OTHER:
        s.append(err_303_page);
        s.append(err_page_tail);
        break;
    // TODO 3xx
    case HTTP_BAD_REQUEST:
        s.append(err_400_page);
        s.append(err_page_tail);
        break;
    case HTTP_UNAUTHORIZED:
        s.append(err_401_page);
        s.append(err_page_tail);
        break;
    case HTTP_FORBINDDEN:
        s.append(err_403_page);
        s.append(err_page_tail);
        break;
    case HTTP_NOT_FOUND:
        s.append(err_404_page);
        s.append(err_page_tail);
        break;
    case HTTP_NOT_ALLOWED:
        s.append(err_405_page);
        s.append(err_page_tail);
        break;
    case HTTP_REQUEST_TIME_OUT:
        s.append(err_408_page);
        s.append(err_page_tail);
        break;
    // TODO 4xx
    case HTTP_INTERNAL_SERVER_ERROR:
        s.append(err_500_page);
        s.append(err_page_tail);
        break;
    case HTTP_NOT_IMPLEMENTED:
        s.append(err_501_page);
        s.append(err_page_tail);
        break;
    case HTTP_BAD_GATEWAY:
        s.append(err_502_page);
        s.append(err_page_tail);
        break;
    case HTTP_GATEWAY_TIME_OUT:
        s.append(err_504_page);
        s.append(err_page_tail);
        break;
    case HTTP_VERSION_NOT_SUPPORT:
        s.append(err_505_page);
        s.append(err_page_tail);
        break;
    case HTTP_INSUFFICIENT_STORAGE:
        s.append(err_507_page);
        s.append(err_page_tail);
        break;
    default:
        LHTTP_ASSERT(false);
        break;
    }
}

} // namespace lighthttp
