#pragma once

#include "dalek_heads.hpp"



namespace http {
namespace http2 {

namespace imple
{
enum {
    fr_start,
    fr_length,
    fr_type,
    fr_flags,
    fr_id,
    fr_payload,
    fr_done
};
}

constexpr unsigned SETTINGS_MAX_FRAME_SIZE = 16384;

// HTTP Frames
// Once the HTTP/2 connection is established, endpoints can begin exchanging frames.

// Frame Format
//
// +-----------------------------------------------+
// |                 Length (24)                   |
// +---------------+---------------+---------------+
// |   Type (8)    |   Flags (8)   |
// +-+-------------+---------------+-------------------------------+
// |R|                 Stream Identifier (31)                      |
// +=+=============================================================+
// |                   Frame Payload (0...)                      ...
// +---------------------------------------------------------------+

// Rfc 7540 https://httpwg.org/specs/rfc7540.html
class frame {
private:

    std::string_view frameStr_;

    // The length of the frame payload expressed as an unsigned 24-bit integer.
    // Values greater than 214 (16,384) MUST NOT be sent unless the receiver has set a larger value for SETTINGS_MAX_FRAME_SIZE.
    unsigned length_;

    // The 8-bit type of the frame. The frame type determines the format and semantics of the frame.
    // Implementations MUST ignore and discard any frame that has a type that is unknown.
    unsigned char type_;

    // An 8-bit field reserved for boolean flags specific to the frame type.
    unsigned char flags_;

    bool R_;

    // Id of stream
    unsigned streamIdentifier_;

    std::string_view framePayload_;

    struct state {
        unsigned p_index;
        unsigned p_state;

        state() :
            p_index(0),
            p_state(imple::fr_start) {
        }

        void reset() {
            p_index = 0;
            p_state = 0;
        }
    };

    state state_;

    int parse(std::string_view f) {
        using namespace imple;

        auto s = state_.p_state;
        auto p = state_.p_index;

        for (; p < f.size() ; ++p)
        {
            unsigned char ch = f[p];
            switch (s) {
            case fr_start:
                s = fr_length;
                break;

            case fr_length: 
                if ( p == 2 )
                {
                    memcpy(&length_, f.data(), 3);
                    if (length_ > SETTINGS_MAX_FRAME_SIZE) 
                    {
                        length_ = SETTINGS_MAX_FRAME_SIZE;
                    }
                    s = fr_type;
                }
                break;

            case fr_type:
                type_ = ch;
                s = fr_flags;
                break;

            case fr_flags:
                flags_ = ch;
                s = fr_id; 
                break;

                /*
                    cast fr_R:
                        break;
                */

            case fr_id:
                if (p == 8)
                {
                    // Length + Type + Flags = 5
                    unsigned char R_byte = *(f.data() + 5);
                    R_byte >> 1;
                    memcpy(&streamIdentifier_, &R_byte, 1);
                    memcpy(&streamIdentifier_ + 1, f.data() + 6, 3);
                    s = fr_payload;
                }
                break;

            case fr_payload:
                if (length_ <= (p - 9) * 8)
                {
                    framePayload_ = std::string_view(f.data() + 9, length_ >> 3);
                    goto done;
                }
                break;

            default:
                assert(false);  
            }
        }

        state_.p_state = s;
        state_.p_index = p;
        return DALEK_AGAIN;
    
    done:
        reset();
        return DALEK_OK;
    }

public:
    frame(std::string_view f):


        frameStr_(f)
    {}

    ~frame() = default;

    frame(const frame &f);
    frame &operator=(const frame &f);

    void SetFrame(std::string_view f)
    {
        frameStr_ = f;
    }

    unsigned length() const 
    {
        return length_;
    }

    unsigned char type() const 
    {
        return type_;
    }

    unsigned char flags() const 
    {
        flags_;
    }

    unsigned streamIdentifier() const 
    {
        return streamIdentifier_;
    }

    std::string_view framePayload() const
    {
        return framePayload_;
    }

    int parse()
    {
        return parse(frameStr_);
    }

    void reset() 
    {
        length_ = 0;
        type_ = 0;
        flags_ = 0;
        streamIdentifier_ = 0;
        framePayload_ = std::string_view();
    }
};

//   
//                             +--------+
//                     send PP |        | recv PP
//                    ,--------|  idle  |--------.
//                   /         |        |         \
//                  v          +--------+          v
//           +----------+          |           +----------+
//           |          |          | send H /  |          |
//    ,------| reserved |          | recv H    | reserved |------.
//    |      | (local)  |          |           | (remote) |      |
//    |      +----------+          v           +----------+      |
//   |          |             +--------+             |          |
//    |          |     recv ES |        | send ES     |          |
//    |   send H |     ,-------|  open  |-------.     | recv H   |
//    |          |    /        |        |        \    |          |
//    |          v   v         +--------+         v   v          |
//    |      +----------+          |           +----------+      |
//    |      |   half   |          |           |   half   |      |
//    |      |  closed  |          | send R /  |  closed  |      |
//    |      | (remote) |          | recv R    | (local)  |      |
//    |      +----------+          |           +----------+      |
//    |           |                |                 |           |
//    |           | send ES /      |       recv ES / |           |
//    |           | send R /       v        send R / |           |
//    |           | recv R     +--------+   recv R   |           |
//    | send R /  `----------->|        |<-----------'  send R / |
//    | recv R                 | closed |               recv R   |
//    `----------------------->|        |<----------------------'
//                             +--------+
//
//       send:   endpoint sends this frame
//       recv:   endpoint receives this frame
//
//       H:  HEADERS frame (with implied CONTINUATIONs)
//       PP: PUSH_PROMISE frame (with implied CONTINUATIONs)
//       ES: END_STREAM flag
//       R:  RST_STREAM frame


class stream {
private:
public:
    stream();
    ~stream();

    stream(const stream &) = delete;
    stream &operator=(const stream &) = delete;

    stream(stream &&);
    stream &operator=(stream &&);
};

}
} // namespace http::http2