#pragma once

#include "dalek_core.hpp"
#include "dalek_mime.hpp"
#include "common/dalek_codes.hpp"

#include <map>
#include <list>
#include <functional>

namespace http
{

    class dalek_http_request;

    class dalek_http_head
    {
    public:
        using HeadHandler = std::function<int(std::string_view&, dalek_http_request&)>;
    private:
        core::Buffer*    buffer_;
        unsigned         key_begin_;
        unsigned         key_end_;
        unsigned         value_begin_;
        unsigned         value_end_;

        void reset()
        {
            buffer_ = nullptr;
            key_begin_ = 0;
            key_end_ = 0;
            value_begin_ = 0;
            value_end_ = 0;
        }

    public:

        dalek_http_head():
            buffer_(nullptr),
            key_begin_(0),
            key_end_(0),
            value_begin_(0),
            value_end_(0)
        {}

        dalek_http_head(const dalek_http_head& h):
            buffer_(h.buffer_),
            key_begin_(h.key_begin_),
            key_end_(h.key_end_),
            value_begin_(h.value_begin_),
            value_end_(h.value_end_)
        {}

        dalek_http_head& operator= (const dalek_http_head& h)
        {
            buffer_ = h.buffer_;
            key_begin_ = h.key_begin_;
            key_end_ = h.key_end_;
            value_begin_ = h.value_begin_;
            value_end_ = h.value_end_;
            return *this;
        }

        dalek_http_head(dalek_http_head&& h):
            buffer_(h.buffer_),
            key_begin_(h.key_begin_),
            key_end_(h.key_end_),
            value_begin_(h.value_begin_),
            value_end_(h.value_end_)
        {
            h.reset();
        }

        dalek_http_head& operator= (dalek_http_head&& h)
        {
            buffer_ = h.buffer_;
            key_begin_ = h.key_begin_;
            key_end_ = h.key_end_;
            value_begin_ = h.value_begin_;
            value_end_ = h.value_end_;
            h.reset();
            return *this;
        }


        void SetBuffer(core::Buffer& b)         { buffer_ = &b; }

        std::string_view key()
        {
            return std::string_view(buffer_->peek() + key_begin_, key_end_ - key_begin_);
        }

        std::string_view value()
        {
            return std::string_view(buffer_->peek() + value_begin_, value_end_ - value_begin_);
        }

        unsigned& key_begin()   { return key_begin_; }
        unsigned& key_end()     { return key_end_; }
        unsigned& value_begin() { return value_begin_; }
        unsigned& value_end()   { return value_end_; }
    };

    class dalek_http_request {
    private:

        struct state {
            unsigned p_state;
            unsigned p_index;
        };

        state state_;

        unsigned uri_start;
        unsigned uri_end;
        unsigned uri_ext;
        unsigned args_start;
        unsigned request_start;
        unsigned request_end;
        unsigned method_end;
        unsigned schema_start;
        unsigned schema_end;
        unsigned host_start;
        unsigned host_end;
        unsigned port_start;
        unsigned port_end;

        unsigned http_minor;
        unsigned http_major;

        unsigned http_header_name_start;
        unsigned http_header_name_end;
        unsigned http_header_value_start;
        unsigned http_header_value_end;
        unsigned header_start;
        unsigned header_end;

        int port;
        int http_version;
        int http_method;
        bool complex_uri;

        std::list<dalek_http_head> heads_;
        core::connection *connection_;



        void handle_request();

        void handle_head();

        void reset() {
            state_.p_index = 0;
            state_.p_state = 0;

            uri_start = 0;
            uri_end = 0;
            uri_ext = 0;
            args_start = 0;
            request_start = 0;
            request_end = 0;
            method_end = 0;
            schema_start = 0;
            schema_end = 0;
            host_start = 0;
            host_end = 0;
            port_start = 0;
            port_end = 0;
            http_minor = 0;
            http_major = 0;

            http_header_name_start = 0;
            http_header_name_end = 0;
            http_header_value_start = 0;
            http_header_value_end = 0;
            header_start = 0;
            header_end = 0;

            port = 0;

            http_version = 0;
            http_method = -1;
            complex_uri = false;
            heads_.clear();
        }

    public:

        dalek_http_request(const dalek_http_request &) = delete;

        dalek_http_request &operator=(const dalek_http_request &) = delete;

        // data begin is the begin of request
        dalek_http_request(core::connection &c, unsigned data_begin) :
                connection_(&c) {
            reset();
            state_.p_index = data_begin;
        }

        dalek_http_request(dalek_http_request &&r) :
                state_(r.state_),
                heads_(std::move(r.heads_)),
                connection_(r.connection_){
            r.reset();
            r.connection_ = nullptr;
        }

        dalek_http_request &operator=(dalek_http_request &&r) {
            connection_ = r.connection_;
            r.connection_ = nullptr;

            state_ = r.state_;

            heads_ = std::move(r.heads_);
            r.reset();
            return *this;
        }

        state &state() { return state_; }

        std::string_view host()
        {
            return std::string_view(connection_->readBuffer().peek() + host_start, host_end - host_start);
        }

        std::string_view port_as_string()
        {
            return std::string_view(connection_->readBuffer().peek() + port_start, port_end - port_start);
        }

        std::string_view schema()
        {
            return std::string_view(connection_->readBuffer().peek() + schema_start, schema_end - schema_start);
        }

        std::string_view uri()
        {
            return std::string_view(connection_->readBuffer().peek() + uri_start, uri_end - uri_start);
        }

        std::string_view method()
        {
            return std::string_view(connection_->readBuffer().peek() + request_start, method_end - request_start);
        }

        int parse_request_line();
        int parse_head_line();
        std::list<dalek_http_head> heads() { return heads_; }
        void finalize_request(int code);

    };



}