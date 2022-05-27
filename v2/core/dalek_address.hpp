#pragma once

#include <string>

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace core {
namespace ip {

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

class address_v4 {
private:
    struct sockaddr_in add_;

public:
    address_v4(const address_v4 &a) :
        add_(a.add_) {
    }

    // TODO (pink)How to do?
    address_v4(address_v4 &&a) :
        add_(a.add_) {
    }

    address_v4 &operator=(const address_v4 &a) {
        add_ = a.add_;
        return *this;
    }

    // TODO (pink)How to do?
    address_v4 &operator=(address_v4 &&a) {
        add_ = a.add_;
        return *this;
    }

    // A empty address
    address_v4() {
        bzero(&add_, sizeof(struct sockaddr_in));
    }

    // For tcp server
    // address_v4(bool loopback, uint16_t port);

    address_v4(const sockaddr_in &addr);
    address_v4(in_addr_t int_addr, uint16_t port);
    address_v4(const char *str_addr, uint16_t port);
    address_v4(const std::string &str_addr, uint16_t port);

    static address_v4 loopback(uint16_t port) {
        // address_v4 port
        return address_v4(INADDR_LOOPBACK, port);
    }

    static address_v4 any(uint16_t port) {
        return address_v4(INADDR_ANY, port);
    }

    static address_v4 from_string(const char *str, uint16_t port) {
        return address_v4(str, port);
    }

    static address_v4 from_string(const std::string &str, uint16_t port) {
        return address_v4(str, port);
    }

    static address_v4 from_string(const std::string &str) {
        auto offset = str.find(':');
        if (offset == str.npos) {
            return address_v4(str, 0);
        } else {
            address_v4 addr;

            int port = atoi(str.c_str() + offset + 1);
            char buffer[1024] = {'\0'};

            memcpy(buffer, str.c_str(), offset);

            inet_pton(AF_INET, buffer, &addr.add_.sin_addr);

            addr.add_.sin_port = htons(port);
            addr.add_.sin_family = AF_INET;
            return addr;
        }
    }

    struct sockaddr_in &sockaddr_in() {
        return add_;
    }

    std::string to_string() const;

    int port() const {
        return ntohs(add_.sin_port);
    }
};

/*
inline address_v4::address_v4(bool loopback, uint16_t port)
{
    bzero(&add_, sizeof add_);
    add_.sin_family = AF_INET;
    in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
    add_.sin_addr.s_addr = htonl(ip);
    add_.sin_port = htons(port);
}
*/

inline address_v4::address_v4(const struct sockaddr_in &addr) :
    add_(addr) {
}

inline address_v4::address_v4(in_addr_t int_add, uint16_t port) {
    bzero(&add_, sizeof add_);
    add_.sin_family = AF_INET;
    add_.sin_addr.s_addr = htonl(int_add);
    add_.sin_port = htons(port);
}

inline address_v4::address_v4(const char *str, uint16_t port) {
    bzero(&add_, sizeof add_);
    // TODO (pink) if return error?
    inet_pton(AF_INET, str, &add_.sin_addr);
    add_.sin_port = htons(port);
    add_.sin_family = AF_INET;
}

inline address_v4::address_v4(const std::string &str, uint16_t port) {
    bzero(&add_, sizeof add_);
    // TODO (pink) if return error?
    inet_pton(AF_INET, str.c_str(), &add_.sin_addr);
    add_.sin_port = htons(port);
    add_.sin_family = AF_INET;
}

inline std::string address_v4::to_string() const {
    char buf[INET_ADDRSTRLEN] = {0};
    const char *res = inet_ntop(AF_INET, (const void *)(&add_.sin_addr), buf, INET_ADDRSTRLEN);
    if (res == nullptr)
        return "";
    else
        return std::string(buf);
}

// TODO
class address_v6 {};

template <typename T>
struct sockaddr *address_cast_sockaddr(T &a);

// Cast from ipv4
template <>
struct sockaddr *address_cast_sockaddr<address_v4>(address_v4 &a) {
    return reinterpret_cast<struct sockaddr *>(&a.sockaddr_in());
}
}
} // namespace core::ip