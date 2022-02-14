// InetAddress
#ifndef PINK_INET_ADDRESS_H
#define PINK_INET_ADDRESS_H

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>

#include <cstring>
#include <string>

namespace pinkx {
namespace net {

// InetAddress, only support ipv4.
class InetAddress {
 private:
  struct sockaddr_in addr4_;

 public:
  explicit InetAddress(uint16_t port = 0, bool lookBack = false);

  InetAddress(char* ip, uint16_t port);

  explicit InetAddress(const struct sockaddr_in& addr) : addr4_(addr) {}

  sa_family_t family() const { return addr4_.sin_family; }

  std::string to_ip() const;
  std::string to_IpPort() const;
  uint16_t port() const;

  struct sockaddr* GetAddr() const {
    return (sockaddr*)(&addr4_);
  }

  // Set hostname to IP and save in result
  static bool resolve(const char* hostName, InetAddress* result);
};

InetAddress::InetAddress(uint16_t port, bool lookBack) {
  memset(&addr4_, 0, sizeof(addr4_));
  addr4_.sin_family = AF_INET;

  auto ip = !lookBack ? INADDR_ANY : INADDR_LOOPBACK;

  addr4_.sin_port = htons(port);
  addr4_.sin_addr.s_addr = htonl(ip);
}

InetAddress::InetAddress(char* ip, uint16_t port) {
  memset(&addr4_, 0, sizeof(addr4_));

  addr4_.sin_family = AF_INET;
  addr4_.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &addr4_.sin_addr) <= 0) {
    // TODO log error
  }
}

// Return IP "a.b.c.d"
std::string InetAddress::to_ip() const {
  char buffer[64];
  if (inet_ntop(AF_INET, &addr4_.sin_addr, buffer,
                static_cast<socklen_t>(sizeof(addr4_))) < 0) {
    // TODO log error
  }
  return buffer;
}

// TODO
std::string InetAddress::to_IpPort() const {
  char buffer[128];
  // ERROR
  return buffer;
}

// Get port
uint16_t InetAddress::port() const {
  return static_cast<uint16_t>(addr4_.sin_port);
}

bool InetAddress::resolve(const char* hostName, InetAddress* result) {
  char buffer[64];
  assert(result != nullptr);
  struct hostent hent;
  struct hostent* he = nullptr;
  int herrno = 0;
  memset(&hent, 0, sizeof(hent));

  int ret = gethostbyname_r(hostName, &hent, buffer, 64, &he, &herrno);

  if (ret == 0 && he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    result->addr4_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  } else {
    if (ret) {
      // TODO log error
    }
    return false;
  }
  return false;
}

}  // namespace net
}  // namespace pinkx
#endif