#pragma once

#include <string_view>
#include <string>

namespace lighturl
{
namespace imple
{

inline bool allow_in_scheme(char ch)
{
    switch (ch) {
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '+':
    case '.':
    case '-':
        return true;
    default:
        return false;
    }

    return false;
}

inline bool allow_in_host(char ch)
{
    switch (ch) {
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '0' ... '9':
    case '.':
        return true;
    default:
        return false;
    }
}

inline bool allow_in_path(char c)
{
    static bool tab[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //   0
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  16
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  32
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  48
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  64
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  80
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  96
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 112
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 128
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 144
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 160
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 176
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 192
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 208
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 224
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  // 240
    };
    return tab[static_cast<unsigned char>(c)];
}

inline bool is_unhex(unsigned char &d, char c)
{
    static signed char tab[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //   0
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  16
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  32
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,           //  48
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  64
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  80
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  96
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 112

        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 240
    };
    d = static_cast<unsigned char>(tab[static_cast<unsigned char>(c)]);
    return d != static_cast<unsigned char>(-1);
}

enum
{
    url_start = 0,
    url_scheme,
    url_scheme_colon,
    url_scheme_colon_slash,
    url_scheme_colon_slash_slash,
    url_account,
    url_password,
    url_host,
    url_port,
    url_abs_path,
    url_query,
    url_params
};

} // namespace imple

enum
{
    Invalid = -1,
    Ok      = 1
};

// Best for http...
class url
{
private:
    std::string scheme_;
    std::string account_;
    std::string password_;
    std::string host_;
    std::size_t port_;
    std::string abs_path_;
    std::string query_;
    std::string params_;

    int state_;

public:
    inline url() :
        port_(0), state_(0)
    {}
    url(const url &) = default;
    ~url()           = default;

    inline url(url &&u) noexcept
        :
        scheme_(std::move(u.scheme_)),
        account_(std::move(u.account_)),
        password_(std::move(u.password_)), host_(std::move(u.host_)),
        port_(std::move(u.port_)), abs_path_(std::move(u.abs_path_)),
        query_(std::move(u.query_)), params_(std::move(u.params_)),
        state_(u.state_)
    {
        u.port_  = 0;
        u.state_ = 0;
    }

    inline url &operator=(url &&u) noexcept
    {
        scheme_   = std::move(u.scheme_);
        account_  = std::move(u.account_);
        password_ = std::move(u.password_);
        host_     = std::move(u.host_);
        port_     = u.port_;
        u.port_   = 0;
        abs_path_ = std::move(u.abs_path_);
        query_    = std::move(u.query_);
        params_   = std::move(u.params_);
        state_    = u.state_;
        u.state_  = 0;
        return *this;
    }

    inline std::string print()
    {
        std::string s;

        if (scheme_.size() > 0) {
            s.append(scheme_);
            s.append("://");
        }

        if (account_.size() > 0) {
            s.append(account_);
            s.push_back(':');
            s.append(password_);
            s.push_back('@');
        }

        if (host_.size() > 0) {
            s.append(host_);
            if (port_ > 0) {
                s.push_back(':');
                s.append(std::to_string(static_cast<int>(port_)));
            }
        }

        if (abs_path_.size() > 0) {
            s.append(abs_path_);
        }

        if (query_.size() > 0) {
            s.push_back('?');
            s.append(query_);
        }

        if (params_.size() > 0) {
            s.push_back('#');
            s.append(params_);
        }

        return s;
    }

    int parse(char ch);
    inline int parse(const char *str, std::size_t len)
    {
        int code = Ok;
        for (auto i = 0; i < len; ++i) {
            if (parse(str[i]) == Invalid) {
                return Invalid;
            }
        }
        return code;
    }

    inline int parse(const std::string &str)
    {
        return parse(str.c_str(), str.size());
    }

    inline int parse(std::string_view str)
    {
        return parse(str.data(), str.size());
    }

    inline std::string &scheme_ref()
    {
        return scheme_;
    }

    inline std::string scheme()
    {
        return scheme_;
    }

    inline std::string &account_ref()
    {
        return account_;
    }

    inline std::string account()
    {
        return account_;
    }

    inline std::string &password_ref()
    {
        return password_;
    }

    inline std::string password()
    {
        return password_;
    }

    inline std::string &host_ref()
    {
        return host_;
    }

    inline std::string host()
    {
        return host_;
    }

    inline std::size_t &port_ref()
    {
        return port_;
    }

    inline std::string &abs_path_ref()
    {
        return abs_path_;
    }

    inline std::string abs_path()
    {
        return abs_path_;
    }

    inline std::string &query_ref()
    {
        return query_;
    }

    inline std::string query()
    {
        return query_;
    }
    
    inline std::string &params_ref()
    {
        return params_;
    }

    inline std::string params()
    {
        return params_;
    }

    inline void reset()
    {
        scheme_.clear();
        account_.clear();
        password_.clear();
        host_.clear();
        port_ = 0;
        abs_path_.clear();
        query_.clear();
        params_.clear();
        state_ = 0;
    }
};

inline int url::parse(char ch)
{
    using namespace imple;

    switch (state_) {
    case url_start:
        if (ch == '/') {
            state_ = url_abs_path;
            abs_path_.push_back(ch);
            break;
        }

        if (!allow_in_scheme(ch)) {
            return Invalid;
        }
        scheme_.push_back(ch);
        state_ = url_scheme;
        break;

    case url_scheme:
        if (ch == ':') {
            state_ = url_scheme_colon;
            break;
        }

        if (!allow_in_scheme(ch)) {
            return Invalid;
        }

        scheme_.push_back(ch);
        break;

    case url_scheme_colon:
        if (ch != '/') {
            return Invalid;
        }
        state_ = url_scheme_colon_slash;
        break;

    case url_scheme_colon_slash:
        if (ch != '/') {
            return Invalid;
        }
        state_ = url_scheme_colon_slash_slash;
        break;

    case url_scheme_colon_slash_slash:
        // Maybe account...
        if (!allow_in_host(ch)) {
            return Invalid;
        }
        account_.push_back(ch);
        state_ = url_account;
        break;

    case url_account:
        if (ch == ':') {
            state_ = url_password;
            break;
        }
        // Should be host
        if (ch == '/') {
            host_ = std::move(account_);
            abs_path_.push_back(ch);
            state_ = url_abs_path;
            break;
        }
        account_.push_back(ch);
        break;

    case url_password:
        // Shoule be port not password
        if (ch == '/') {
            host_ = std::move(account_);
            port_ = 0;
            for (auto i : password_) {
                if (i < '0' || i > '9')
                    return Invalid;
                port_ = port_ * 10 + i - '0';
            }
            password_.clear();
            abs_path_.push_back(ch);
            state_ = url_abs_path;
            break;
        }

        // Ok this is password
        if (ch == '@') {
            state_ = url_host;
            break;
        }

        password_.push_back(ch);
        break;

    case url_host:
        if (ch == ':') {
            port_  = 0;
            state_ = url_port;
            break;
        }

        if (ch == '/') {
            abs_path_.push_back(ch);
            state_ = url_abs_path;
            break;
        }

        if (!allow_in_host(ch)) {
            return Invalid;
        }

        host_.push_back(ch);
        break;

    case url_port:
        if (ch == '/') {
            state_ = url_abs_path;
            abs_path_.push_back(ch);
            break;
        }

        if (ch < '0' || ch > '9') {
            return Invalid;
        }

        port_ = port_ * 10 + ch - '0';
        break;

    case url_abs_path:
        if (ch == '?') {
            state_ = url_query;
            break;
        }

        if (ch == '#') {
            state_ = url_params;
            break;
        }

        if (!allow_in_path(ch)) {
            return Invalid;
        }
        abs_path_.push_back(ch);
        break;

    case url_query:
        if (ch == '#') {
            state_ = url_params;
            break;
        }

        if (!allow_in_path(ch)) {
            return Invalid;
        }

        query_.push_back(ch);
        break;

    case url_params:
        if (!allow_in_path(ch)) {
            return Invalid;
        }
        params_.push_back(ch);
        break;
    }

    return Ok;
}

} // namespace lighturl
