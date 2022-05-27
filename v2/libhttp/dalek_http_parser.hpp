#pragma  once

#include "dalek_http_request.hpp"

#define CR '\r'
#define LF '\n'

namespace http
{
    namespace imple
    {
        enum {
            rl_begin = 0,
            rl_method,
            rl_uri_start,
            rl_schema,
            rl_schema_slash,
            rl_schema_slash_slash,
            rl_host_begin,
            rl_host,
            rl_host_end,
            rl_host_ip_iteral,
            rl_port,
            rl_after_slash_in_uri,
            rl_check_uri,
            rl_uri,
            rl_http9,
            rl_http_h,
            rl_http_ht,
            rl_http_htt,
            rl_http_http,
            rl_first_major_digit,
            rl_major_digit,
            rl_first_minor_digit,
            rl_minor_digit,
            rl_spaces_after_digit,
            rl_almost_done,
            hl_start,
            hl_name,
            hl_space_before_colon,
            hl_colon,
            hl_space_before_value,
            hl_value,
            hl_almost_done,
            hl_header_almost_done
        };

        inline bool dalek_cmp3(char* m, char* n)
        {
            return m[0] == n[0] && m[1] == n[1] && m[2] == n[2];
        }

        inline bool dalek_cmp4(char* m, char* n)
        {
            return dalek_cmp3(m, n) && m[3] == n[3];
        }

        inline bool dalek_cmp5(char* m, char* n)
        {
            return dalek_cmp4(m, n) && m[4] == n[4];
        }

        inline bool dalek_cmp6(char* m, char* n)
        {
            return dalek_cmp5(m, n) && m[5] == n[5];
        }

        inline bool dalek_cmp7(char* m, char* n)
        {
            return dalek_cmp6(m, n) && m[6] == n[6];
        }

        inline bool dalek_cmp8(char* m, char* n)
        {
            return dalek_cmp7(m, n) && m[7] == n[7];
        }

        inline bool dalek_cmp9(char* m, char* n)
        {
            return dalek_cmp8(m, n) && m[8] == n[8];
        }
    }


    // TODO(pink): check uri
    inline int http::dalek_http_request::parse_request_line()
    {
        using namespace imple;
        auto state = state_.p_state;
        int i = state_.p_index;

        if (connection_ == nullptr)
            throw std::runtime_error {"'void http::dalek_http_request::parse_request_line()', connection is nullptr"};

        char* buffer = connection_->readBuffer().peek();
        for (; i < connection_->readBuffer().size(); ++i)
        {
            char ch = buffer[i];
            switch (state) {

            case rl_begin:

                request_start = i;
                if (ch == CR) {
                    break;
                }

                if (ch < 'A' || ch > 'Z') {
                    return DALEK_HTTP_PARSE_INVALID_METHOD;
                }

                state = rl_method;
                break;

            case rl_method:

                if (ch == ' ') {
                    method_end = i;
                    char *m = buffer + request_start;
                    switch (i - request_start) {
                    case 3:
                        if (dalek_cmp3(m, (char*)"GET")) {
                            http_method = HTTP_GET;
                        }
                        if (dalek_cmp3(m, (char*)"PUT")) {
                            http_method = HTTP_PUT;
                        }
                        break;
                    case 4:
                        if (dalek_cmp4(m, (char*)"POST")) {
                            http_method = HTTP_POST;
                        }
                        if (dalek_cmp4(m, (char*)"HEAD")) {
                            http_method = HTTP_HEAD;
                        }
                        break;
                    case 5:
                        if (dalek_cmp5(m, (char*)"PATCH")) {
                            http_method = HTTP_PATCH;
                        }
                        if (dalek_cmp5(m, (char*)"TRACE")) {
                            http_method = HTTP_TRACE;
                        }
                        break;
                    case 6:
                        if (dalek_cmp6(m, (char*)"DELETE")) {
                            http_method = HTTP_DELETE;
                        }
                        break;
                    case 7:
                        if (dalek_cmp7(m, (char*)"OPTIONS")) {
                            http_method = HTTP_OPTIONS;
                        }
                        if (dalek_cmp7(m, (char*)"CONNECT")) {
                            http_method = HTTP_CONNECT;
                        }
                        break;
                    }   // switch
                    state = rl_uri_start;
                }
                break;  // rl_method

            case rl_uri_start: {
                // GET /index.htm
                // Parse abs path
                if (ch == '/') {
                    uri_start = i;
                    // abs path
                    state = rl_after_slash_in_uri;
                    break;
                }

                char c = (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch;
                // Parse schema
                if (c >= 'a' && c <= 'z') {
                    schema_start = i;
                    state = rl_schema;
                    break;
                }

                switch (ch) {
                case ' ':
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }

                break;  // rl_uri_start
            }

            case rl_schema: {
                char c = (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch;
                if (c >= 'a' && c <= 'z') {
                    break;
                }

                if ((ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '.') {
                    break;
                }

                switch (ch) {
                case ':':
                    schema_end = i;
                    state = rl_schema_slash;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;  // rl_schema
            }

            // GET http:/
            case rl_schema_slash:
                switch (ch) {
                case '/':
                    state = rl_schema_slash_slash;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            // GET http://
            case rl_schema_slash_slash:
                switch (ch) {
                case '/':
                    state = rl_host_begin;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            case rl_host_begin:
                host_start = i;
                if (ch == '[') {
                    // TODO
                }
                state = rl_host;
                break;

            case rl_host: {
                char c = (ch >= 'A' && ch <= 'Z') ? ch = ch - 'A' + 'a' : ch;

                if (c >= 'a' && c <= 'z') {
                    break;
                }

                if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {
                    break;
                }

                // www.pink.cn/root/index.htm
                // path
                if (ch == '/')
                {
                    uri_start = i;
                    state = rl_after_slash_in_uri;
                    break;
                }

                // 192.168.2.1:8000
                if (ch == ':')
                {
                    host_end = i;
                    port = 0;
                    port_start = i + 1;
                    state = rl_port;
                    break;
                }

                if (ch == ' ')
                {
                    host_end = i;
                    uri_start = schema_end + 1;
                    uri_end = schema_end +2;
                    state = rl_http9;
                    break;
                }

                break;
            }

            /*
            case rl_host_end:
                switch (ch) {
                case '/':
                    uri_start = i + 1;
                    state = rl_after_slash_in_uri;
                    break;
                case '?':
                    uri_start = i;
                    args_start = i + 1;
                    state = rl_uri;
                    break;
                case ' ':
                    uri_start = schema_end + 1;
                    uri_end = schema_end +2;
                    state = rl_http9;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;
            */

            case rl_port:
                switch (ch) {
                case '0'...'9':
                    port += ch - '0';
                    break;
                case ' ':
                    port_end = i;
                    uri_start = schema_end + 1;
                    uri_end = schema_end +2;
                    state = rl_http9;
                    break;

                    // www.pink.cn:8000/hello/hw.htm
                case '/':
                    port_end = i;
                    uri_start = i;
                    state = rl_after_slash_in_uri;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            // For abs path
            case rl_after_slash_in_uri:

                switch (ch) {
                case ' ':
                    uri_end = i;
                    state = rl_http9;
                    break;
                case CR:
                    uri_end = i;
                    http_minor = 9;
                    state = rl_almost_done;
                    break;
                case LF:
                    uri_end = i;
                    http_minor = 9;
                    goto done;
                case '/':
                    complex_uri = true;
                    state = rl_uri;
                    break;
                default:
                    if (ch < 0x20 || ch == 0x7f) {
                        return DALEK_HTTP_PARSE_INVALID_REQEUST;
                    }
                    state = rl_uri;
                    break;
                }
                break; // rl_after_slash_in_uri

            case rl_uri:
                switch (ch) {
                case ' ':
                    uri_end = i;
                    state = rl_http9;
                    break;
                case CR:
                    uri_end = i;
                    http_minor = 9;
                    break;
                case LF:
                    uri_end = i;
                    http_minor = 9;
                    goto done;
                case '?':
                    complex_uri = true;
                    break;
                case '#':
                    complex_uri = true;
                    break;
                default:
                    if (ch < 0x20 || ch == 0x7f) {
                        return DALEK_HTTP_PARSE_INVALID_REQEUST;
                    }
                    break;
                }
                break;

            case rl_http9:
                switch (ch) {
                case ' ':
                    break;
                case CR:
                    state = rl_almost_done;
                    http_minor = 9;
                    break;
                case LF:
                    http_minor = 9;
                    goto done;
                case 'H':
                    state = rl_http_h;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break; // rl_http09

            case rl_http_h:
                switch (ch) {
                case 'T':
                    state = rl_http_ht;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            case rl_http_ht:
                switch (ch) {
                case 'T':
                    state = rl_http_htt;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            case rl_http_htt:
                switch (ch) {
                case 'P':
                    state = rl_http_http;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            case rl_http_http:
                switch (ch) {
                case '/':
                    state = rl_first_major_digit;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            case rl_first_major_digit:
                if (ch < '1' || ch > '9') {
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }

                http_major = ch - '0';

                if (http_major > 1) {
                    return DALEK_HTTP_PARSE_INVALID_HTTP_VERSION;
                }

                state = rl_major_digit;
                break;

            case rl_major_digit:
                if (ch == '.') {
                    state = rl_first_minor_digit;
                    break;
                }

                if (ch < '0' || ch > '9') {
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }

                http_major = http_major * 10 + (ch - '0');

                if (http_major > 1) {
                    return DALEK_HTTP_PARSE_INVALID_HTTP_VERSION;
                }
                break;  // rl_major_digit

            case rl_first_minor_digit:
                if (ch < '0' || ch > '9') {
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }

                http_minor = ch - '0';
                state = rl_minor_digit;
                break;

            case rl_minor_digit:
                if (ch == CR) {
                    state = rl_almost_done;
                    break;
                }
                if (ch == LF) {
                    goto done;
                }
                if (ch == ' ') {
                    state = rl_spaces_after_digit;
                    break;
                }
                if (ch < '0' || ch > '9') {
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }

                if (http_minor > 99) {
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                http_minor = http_minor * 10 + (ch - '0');
                break;

            case rl_spaces_after_digit:

                switch (ch) {
                case ' ':
                    break;
                case CR:
                    state = rl_almost_done;
                    break;
                case LF:
                    goto done;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;  // rl_spaces_after_digit

            case rl_almost_done:
                request_end = i - 1;
                switch (ch) {
                case LF:
                    goto done;
                default:
                    return DALEK_HTTP_PARSE_INVALID_REQEUST;
                }
                break;

            }   // switch state
        }   // loop

        state_.p_index = i;
        state_.p_state = state;
        return DALEK_AGAIN;
    done:
        state_.p_index = i + 1;
        state_.p_state = hl_start;

        if (request_end = 0) {
            request_end = i;
        }

        http_version = http_major * 1000 + http_minor;


        if (http_version == 9 && http_method != HTTP_GET) {
            return DALEK_HTTP_PARSE_INVALID_REQEUST;
        }

        return DALEK_OK;
    }

    inline int dalek_http_request::parse_head_line()
    {
        using namespace imple;

        dalek_http_head header;

        auto state = state_.p_state;
        int i = state_.p_index;
        char* buffer = connection_->readBuffer().peek();
        for (;i < connection_->readBuffer().size(); ++i)
        {
            char ch = buffer[i];
            switch (state)
            {
            case hl_start:

                http_header_name_start = i;
                header_start = i;

                switch (ch) {
                case CR:
                    header_end = i;
                    state =  hl_header_almost_done;
                    break;
                case LF:
                    header_end = i;
                    goto header_done;
                default:
                    state = hl_name;
                    break;
                }

                break;

            case hl_name:

                if (ch == ':')
                {
                    http_header_name_end = i;
                    state = hl_colon;
                }

                if (ch == ' ')
                {
                    http_header_name_end = i;
                    state = hl_space_before_colon;
                }

                if (ch == CR)
                {
                    http_header_name_end = i;
                    header_end = i;
                    state = hl_almost_done;
                }

                if (ch == LF)
                {
                    header_end = i;
                    http_header_name_end = i;
                    goto done;
                }

                // TODO(pink)upstream

                break;

            case hl_space_before_colon:

                switch (ch) {
                case ' ':
                    break;
                case ':':
                    state = hl_colon;
                    break;
                default:
                    return DALEK_HTTP_PARSE_INVALID_HEAD;
                    break;
                }

                break;

            case hl_colon:

                switch (ch) {
                case ' ':
                    state = hl_space_before_value;
                    break;
                case CR:
                case LF:
                    return DALEK_HTTP_PARSE_INVALID_HEAD;
                default:
                    state = hl_value;
                    http_header_value_start = i;
                    break;
                }

                break;

            case hl_space_before_value:

                switch (ch) {
                case ' ':
                    break;
                case CR:
                    header_start = i;
                    header_end = i;
                    state = hl_almost_done;
                    break;
                case LF:
                    header_start = i;
                    header_end = i;
                    goto done;
                case ':':
                    return DALEK_HTTP_PARSE_INVALID_HEAD;
                default:
                    state = hl_value;
                    http_header_value_start = i;
                    break;
                }
                break;

            case hl_value:

                switch (ch) {
                case CR:
                    http_header_value_end = i;
                    state = hl_almost_done;
                    break;
                case LF:
                    goto done;
                default:
                    break;
                }

                break;  // hl_value

            case hl_almost_done:

                switch (ch) {
                case LF:
                    header.key_begin() = http_header_name_start;
                    header.key_end() = http_header_name_end;
                    header.value_begin() = http_header_value_start;
                    header.value_end() = http_header_value_end;
                    header.SetBuffer(connection_->readBuffer());
                    heads_.emplace_back(header);
                    goto done;
                default:
                    return DALEK_HTTP_PARSE_INVALID_HEAD;
                }

                break;  // hl_almost_done

            case hl_header_almost_done:

                switch (ch) {
                case LF:
                    goto header_done;
                default:
                    return DALEK_HTTP_PARSE_INVALID_HEAD;
                }

                break;
            }


        }

        state_.p_state = state;
        state_.p_index = i;
        return DALEK_AGAIN;

        done:

            state_.p_index = i + 1;
            state_.p_state = hl_start;
            return DALEK_OK;

        header_done:

            state_.p_index = i + 1;
            state_.p_state = rl_begin;
            return DALEK_HTTP_PARSE_HEADER_DONE;
    }


}