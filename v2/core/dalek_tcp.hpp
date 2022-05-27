#pragma once

#include "dalek_address.hpp"

#include <memory>
#include <cstdlib>

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>

namespace core {
namespace tcp {
// Basic tcp socket
class tcpsocket;
// Tcp connection
class connection;
// Tcp listener
class listener;

class tcpsocket {
private:
    int fd_;
    ip::address_v4 address_;

public:
    static constexpr int MAXCONN = 4096;

    tcpsocket(const tcpsocket &) = delete;
    tcpsocket &operator=(const tcpsocket &) = delete;

    tcpsocket(tcpsocket &&t) :
        fd_(t.fd_),
        address_(t.address_) {
        t.fd_ = -1;
    }

    tcpsocket &operator=(tcpsocket &&t) {
        fd_ = t.fd_;
        address_ = t.address_;
        t.fd_ = -1;
        return *this;
    }

    // For Connecter
    tcpsocket();

    // For Client
    tcpsocket(int fd);
    tcpsocket(int fd, const ip::address_v4 &add);

    // For Server
    tcpsocket(const ip::address_v4 &add);
    tcpsocket(const char *str, int port);
    tcpsocket(const std::string &str, int port);

    int Bind();
    int Listen();
    tcpsocket Accept();
    int AcceptByfd();

    int Connect(ip::address_v4 &add);

    int UseNagle(bool b);
    int SetNonBlock();

    int Fd() const {
        return fd_;
    }
    ip::address_v4 &Address() {
        return address_;
    }

    void Shutdown(int how) {
        ::shutdown(fd_, how);
    }
    void Close() {
        ::close(fd_);
    }
};

inline tcpsocket::tcpsocket() :
    fd_(socket(AF_INET, SOCK_STREAM, 0)),
    address_() {
}

inline tcpsocket::tcpsocket(int fd) :
    fd_(fd),
    address_() {
}

inline tcpsocket::tcpsocket(int fd, const ip::address_v4 &add) :
    fd_(fd),
    address_(add) {
}

inline tcpsocket::tcpsocket(const ip::address_v4 &add) :
    fd_(socket(AF_INET, SOCK_STREAM, 0)),
    address_(add) {
    // TODO (pink) If fd is -1?
    // ASSERT(fd_ < 0);
    assert(fd_ > 0);
}

inline tcpsocket::tcpsocket(const char *str, int port) :
    fd_(socket(AF_INET, SOCK_STREAM, 0)),
    address_(str, port) {
    // TODO (pink) If fd is -1?
    // ASSERT(fd_ < 0);
    assert(fd_ > 0);
}

inline tcpsocket::tcpsocket(const std::string &str, int port) :
    fd_(socket(AF_INET, SOCK_STREAM, 0)),
    address_(str.c_str(), port) {
    // TODO (pink) If fd is -1?
    // ASSERT(fd_ < 0);
    assert(fd_ > 0);
}

inline int tcpsocket::Bind() {
    return ::bind(fd_, address_cast_sockaddr(address_), sizeof(sockaddr_in));
}

inline int tcpsocket::Listen() {
    return ::listen(fd_, MAXCONN);
}

inline tcpsocket tcpsocket::Accept() {
    ip::address_v4 addr;
    socklen_t len{sizeof(struct sockaddr_in)};
    int fd = ::accept(fd_, address_cast_sockaddr(addr), &len);
    return tcpsocket(fd, addr);
}

inline int tcpsocket::Connect(ip::address_v4 &add) {
    return ::connect(fd_, address_cast_sockaddr(add), sizeof(sockaddr));
}

inline int tcpsocket::AcceptByfd() {
    ip::address_v4 addr;
    socklen_t len{sizeof(struct sockaddr_in)};
    return ::accept(fd_, address_cast_sockaddr(addr), &len);
}

inline int tcpsocket::UseNagle(bool b) {
    int op = b ? 0 : 1;
    return setsockopt(fd_, IPPROTO_TCP, O_NDELAY, (char *)&op, sizeof op);
}

inline int tcpsocket::SetNonBlock() {
    int state = fcntl(fd_, F_GETFD);
    return fcntl(fd_, F_SETFD, state | O_NONBLOCK);
}
}
} // namespace core::tcp