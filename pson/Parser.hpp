#ifndef PINK_PSON_PARSER_HPP
#define PINK_PSON_PARSER_HPP
#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <string>

#include "Value.hpp"

#if __cplusplus >= 201703L
#include <string_view>
#define PSON_CXX17
#endif

#include <math.h>

namespace pson {

    class Object;
    class Value;



    class Parser {
    private:

        struct state {
            char* data_;
            size_t size_;
            size_t offset_;

            state(char* data, size_t size):
                data_(data),
                size_(size),
                offset_(0) {}

           state():
                data_(nullptr),
                size_(0),
                offset_(0) {}

           void reset() { data_ = nullptr; size_ = 0; offset_ = 0; }
           void set(char* data, size_t len) { reset(); data_ = data; size_ = len; }
        };

        state state_;

        // Insignificant whitespace is allowed before or after any of the six
        //   structural characters.
        //
        //      ws = *(
        //                %x20 /              ; Space
        //                %x09 /              ; Horizontal tab
        //                %x0A /              ; Line feed or New line
        //                %x0D                ; Carriage return
        //            )
        void skip_write_blank();

        // [name] : true
        bool parse_true(Value& v);

        // [name] : false
        bool parse_false(Value& v);

        // [name] : null
        bool parse_null(Value& v);

        // :)
        bool parse_number(Value& v);

        // \uXXXX
        bool parse_hex4(Value& v);

        bool parse_unicode(Value& v);

        // Only parse string
        bool parse_string_row(String& s);

        // parse value string
        bool parse_string(Value& v);

        // [name]: [val1, val2, val3......]
        bool parse_array(Value& v);

        bool parse_object(Value& v);

        bool parse_value(Value& v);
    public:

        Parser(const std::string& data):
            state_(const_cast<char*>(data.data()) , data.size()) {}

        Parser(char* data, size_t size):
            state_(const_cast<char*>(data ), size) {}

        bool Parse(Value& v);

        void Reset() { state_.reset(); }
        void SetData(char* data, size_t len) { Reset(); state_.set(data, len); }
        void SetData(std::string& data) { Reset(); state_.set(const_cast<char*>(data.data()), data.size()); }

    };

    void Parser::skip_write_blank()
    {
        auto& offset = state_.offset_;
        auto d = state_.data_;
        while(offset < state_.size_)
        {
            auto ch = d[offset];
            if (ch == 0x20 || ch == 0x09 || ch == 0x0A || ch == 0x0D ) offset++;
            else break;
        }
    }

    bool Parser::parse_true(Value &v)
    {
        auto& offset = state_.offset_;
        auto d = state_.data_;
        PSON_ASSERT(d[offset] == 't');

        if (offset + 3 >= state_.size_ ||
            d[offset + 1] != 'r' ||
            d[offset + 2] != 'u' ||
            d[offset + 3] != 'e') return false;

        offset += 4;
        v.ImportBool(true);

        return true;
    }

    bool Parser::parse_false(Value &v)
    {
        auto& offset = state_.offset_;
        auto d = state_.data_;
        PSON_ASSERT(d[offset] == 'f');

        if (offset + 4 >= state_.size_ ||
            d[offset + 1] != 'a' ||
            d[offset + 2] != 'l' ||
            d[offset + 3] != 's' ||
            d[offset + 4] != 'e') return false;

        offset += 5;
        v.ImportBool(false);

        return true;
    }

    bool Parser::parse_null(Value &v)
    {
        auto& offset = state_.offset_;
        auto d = state_.data_;
        PSON_ASSERT(d[offset] == 'n');

        if (offset + 3 >= state_.size_ ||
            d[offset + 1] != 'u' ||
            d[offset + 2] != 'l' ||
            d[offset + 3] != 'l') return false;

        offset += 4;
        v.ImportNull();

        return true;
    }

    bool Parser::parse_number(Value &v)
    {
#define ISDIGIT(ch) (ch >= '0' && ch <= '9')
#define ISONETONINE(ch) (ch >= '1' && ch <= '9')

        const char* begin = state_.data_ + state_.offset_;
        char* end = const_cast<char*>(begin);

        if (*end == '-') end++;
        if (*end == '0') end++;
        else {
            if (!ISONETONINE(*end)) return false;   // 00.1
            for(;ISDIGIT(*end); end++);
        }
        if (*end == '.')
        {
            end++;
            if (!ISDIGIT(*end)) return false;       // 0.e1
            for(;ISDIGIT(*end); end++);
        }
        if (*end == 'e' || *end == 'E')
        {
            end++;
            if (*end == '+' || *end == '-') end++;
            if (!ISDIGIT(*end)) return false;
            for(;ISDIGIT(*end); end++);
        }
        errno = 0;
        Number res = strtod(begin , nullptr);
        if (errno == ERANGE || res == HUGE_VAL || res == -HUGE_VAL) return false;       // Number too big !
        v.ImportNumber(res);
        state_.offset_ += end - begin;
        return true;

#undef ISONETONINE
#undef ISDIGIT

    }

    bool Parser::parse_hex4(Value &v)
    {

    }

    bool Parser::parse_unicode(Value &v)
    {

    }

    bool Parser::parse_string_row(String& s)
    {
        auto& offset = state_.offset_;
        auto d = state_.data_;
        PSON_ASSERT(d[offset] == '\"');
        ++offset;
        while (true)
        {
            if (offset >= state_.size_) return false;
            char ch = d[offset];
            switch (ch) {
                case '\"': {offset++; return true; }
                case '\0': return false;
                default: {
                    s.push_back(ch);
                    ++offset;
                }
            }
        }
        
    }

    bool Parser::parse_string(Value &v)
    {
        String s;
        if (!parse_string_row(s)) return false;

        v.ImportString(std::move(s));       // Zero Overhead
        return true;
    }

    bool Parser::parse_array(Value &v)
    {
        auto& offset = state_.offset_;
        char* d = state_.data_;
        PSON_ASSERT(d[offset] == '[');
        ++offset;
        skip_write_blank();
        if (offset >= state_.size_) return false;

        Array array;
        v.ImportArray(array);
        
        if (d[offset] == ']')
        {
            offset++;
            return true;
        }

        while(true)
        {
            Value* val = new Value();
            if (!Parse(*val)) return false;
            v.AsArray().Push(val);
            skip_write_blank();

            if (d[offset] == ',')
                offset++;
            else if (d[offset] == ']')
            {
                offset++;
                return true;
            }
            else
                return false;

        }
    }

    bool Parser::parse_object(Value &v)
    {
        auto& offset = state_.offset_;
        char* d = state_.data_;
        PSON_ASSERT(d[offset] == '{');
        ++offset;
        skip_write_blank();

        if (offset >= state_.size_) {
            return false;
        }

        Object obj;
        v.ImportObject(obj);

        if (d[offset] == '}')
        {
            offset++;
            return true;
        }

        while(true)
        {
          String name;
          if (!parse_string_row(name)) return false;
          skip_write_blank();
          
          Value* val = new Value();
          char ch = d[offset];
          if (d[offset] != ':') return false;
          ++offset;

          auto ret = Parse(*val);
          if (!ret) return false;
          v.AsObject().Insert(std::move(name), val);

          skip_write_blank();
          if (d[offset] == ',') {
            ++offset;
            skip_write_blank();
            }
            else if (d[offset] == '}')
            {
                ++offset;
                return true;
            }
            else if (offset >= state_.size_)
            {
                return false;
            }
        }
    }

    bool Parser::parse_value(Value& v)
    {
        char ch = state_.data_[state_.offset_];
        switch (ch) {
            case 't':
                return parse_true(v);
            case 'f':
                return parse_false(v);
            case 'n':
                return parse_null(v);
            case '\"':
                return parse_string(v);
            case '[':
                return parse_array(v);
            case '{':
                return parse_object(v);
            default:
                return parse_number(v);
        }
        // Make happy
        return false;
    }

    bool Parser::Parse(Value& v)
    {
        skip_write_blank();
        return parse_value(v);
    }

}   // namespace pson

#undef PSON_ASSERT

#endif