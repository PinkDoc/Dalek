// Socket
#ifndef PINK_SOCKET_H
#define PINK_SOCKET_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#include "InetAddress.h"

namespace pinkx {
namespace net {

#define MAX_LISTEN 4096

// Create non block socket
static int CreateNonBlockSocket() {
  int ret =
      socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  return ret;
}

// Return old state, and set state to nonblocking
static int SetNonBlocking(int fd) {
  int old_state = fcntl(fd, F_GETFL);
  int new_state = fcntl(fd, F_SETFL, old_state | O_NONBLOCK);
  return old_state;
}

class Socket {
 private:
  int fd_;

 public:
  Socket(int fd_);
  ~Socket();

  int fd() const;

  void KeepAilve(bool keep);

  void UseNagle(bool use);

  void SetBindAddress(const InetAddress& address);

  void SetReuseAddr(bool resuse);

  void SetReusePort(bool reuse);  // mult process

  int listen();

  int accept(InetAddress& address);

  void SetFd(int fd);
};

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {}

int Socket::fd() const { return fd_; }

void Socket::KeepAilve(bool keep) {
  int optval = keep ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
}

void Socket::SetBindAddress(const InetAddress& address) {
  bind(fd_, address.GetAddr(), sizeof(sockaddr));
}

void Socket::SetReuseAddr(bool reuse) {
  int on = reuse ? 1 : 0;
  setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
}

void Socket::SetReusePort(bool reuse) {
  int on = reuse ? 1 : 0;
  setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &on, sizeof on);
}

int Socket::listen() {
  int ret = ::listen(fd_, MAX_LISTEN);
  return ret;
}

void Socket::UseNagle(bool use) {
  int on = use ? 0 : 1;
  setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &on, sizeof on);
}

// accept and set address to address
int Socket::accept(InetAddress& address) {
  socklen_t len;
  int ret = ::accept(fd_, address.GetAddr(), &len);
  return ret;
}

void Socket::SetFd(int fd) { fd_ = fd; }

}  // namespace net
}  // namespace pinkx

#endif