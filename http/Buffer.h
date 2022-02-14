// buffer
#ifndef PINK_NBUFFER_H
#define PINK_NBUFFER_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <vector>

namespace pinkx {

class Buffer {
 private:
  std::vector<char> buffer_;
  size_t endIndex_;

  void resize();

 public:
  Buffer();

  char* peek() const { return const_cast<char*>(buffer_.data()); }
  char* writeBegin() const { return peek() + endIndex_; }
  size_t size() const { return endIndex_; }

  size_t capacity() const { return buffer_.capacity(); }

  size_t writeable() { return capacity() - size(); }

  void reset() {
    endIndex_ = 0;
    buffer_.clear();
  }

  int readFromFd(int fd);
  int sendToFd(int fd);
  void append(char* buf, size_t len);

  void sprintf(char* format, ...);
};

Buffer::Buffer() : endIndex_(0), buffer_(1024, 0) {}

void Buffer::resize() { buffer_.resize(capacity() * 2); }

int Buffer::readFromFd(int fd) {
  if (fd < 0) {
    return -1;
  }

  size_t temp = writeable();

  iovec io[2];
  char stack_buf[65536];  // Stack
  io[0].iov_base = writeBegin();
  io[0].iov_len = temp;
  io[1].iov_base = stack_buf;
  io[1].iov_len = 65536;
  ssize_t n = readv(fd, io, 2);

  if (n > temp)  // more need toi copy
  {
    endIndex_ = capacity();
    append(stack_buf, n - temp);
  } else
    endIndex_ += n;
  return n;
}

void Buffer::append(char* buf, size_t len) {
  if (writeable() < len) {
    resize();
  }
  std::copy(buf, buf + len, writeBegin());

  endIndex_ += len;
}

// It is safe
void Buffer::sprintf(char* format, ...) {
  char* begin = buffer_.data();
  va_list args;
  va_start(args, format);
  int offset = ::vsnprintf(writeBegin(), capacity() - endIndex_, format, args);
  va_end(args);
  endIndex_ += offset;
}

int Buffer::sendToFd(int fd) {
  /*
  if (fd == -1) return true;
  int ret = send(fd, buffer_.data(), endIndex_, 0);
  if (ret == 0 || ret == endIndex_)
  {
      endIndex_ -= ret;
      return true;
  } else {
      if (ret > 0)
          endIndex_ -= ret;
      return false;
  }*/
  if (fd == -1) return 1;
  int ret = send(fd, buffer_.data(), endIndex_, 0);
  if (ret > 0) endIndex_ -= ret;
  return ret;
}

}  // namespace pinkx
#endif