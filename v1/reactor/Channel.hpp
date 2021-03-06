// channel
#ifndef PINK_CHANNEL_HPP
#define PINK_CHANNEL_HPP

#include <sys/epoll.h>

#include <functional>

#include "EventLoop.hpp"
#include "base.hpp"

namespace pinkx {

class EventLoop;

class Channel {
 private:
  int fd_;
  EventLoop* looper_;

  // call back
  std::function<void()> readCallBack;     // epoll in
  std::function<void()> writeCallBack;    // epoll out
  std::function<void()> errorCallBack;    // epollerr epollnvl
  std::function<void()> timeOutCallBack;  // time out
  std::function<void()> closeCallBack;    // epollrdhup epollhup

  int event_;      // Happending
  int revent_;     // User
  int happended_;  // new or deleted or added
 public:
  Channel(EventLoop& looper, int fd);
  void CallBack();

  void SetReadCallBack(std::function<void()> cb);
  void SetWriteCallBack(std::function<void()> cb);
  void SetErrorCallBack(std::function<void()> cb);
  void SetTimeOutCallBack(std::function<void()> cb);

  inline int fd() const { return fd_; }

  static const int IsNew;
  static const int IsDeleted;
  static const int IsAdded;

  void EnableRead();
  void EnableWrite();
  void DisableRead();
  void DisableWrite();

  void SetRevent(int revt);
  int event() const;
  int happended() const;
  void SetDeleted();
  void SetAdded();

  void TimeOutCallBack();

  void SetFd(int fd);
  void SetEventLooper(EventLoop* looper);
};

const int Channel::IsNew = 0;
const int Channel::IsDeleted = -1;
const int Channel::IsAdded = 1;

inline Channel::Channel(EventLoop& looper, int fd)
    : looper_(&looper), fd_(fd), event_(0), revent_(0), happended_(0) {}

inline void Channel::CallBack() {
  if ((revent_ & (EPOLLHUP | EPOLLRDHUP) && (!revent_ & EPOLLIN))) {
    if (closeCallBack) closeCallBack();
  }

  if (revent_ & (EPOLLIN | EPOLLPRI)) {
    if (readCallBack) readCallBack();
  }

  if (revent_ & (EPOLLOUT)) {
    if (writeCallBack) writeCallBack();
  }

  if (revent_ & (EPOLLERR)) {
    if (errorCallBack) errorCallBack();
  }
}

inline void Channel::SetReadCallBack(std::function<void()> cb) { readCallBack = cb; }

inline void Channel::SetWriteCallBack(std::function<void()> cb) { writeCallBack = cb; }

inline void Channel::SetErrorCallBack(std::function<void()> cb) { errorCallBack = cb; }

inline void Channel::SetTimeOutCallBack(std::function<void()> cb) { timeOutCallBack = cb; }

inline void Channel::EnableRead() { event_ |= (EPOLLIN); }

inline void Channel::EnableWrite() { event_ |= EPOLLOUT; }

inline void Channel::DisableRead() { event_ &= ~EPOLLIN; }

inline void Channel::DisableWrite() { event_ &= ~EPOLLOUT; }

inline void Channel::SetRevent(int op) {
  revent_ = 0;
  revent_ |= op;
}

inline int Channel::event() const { return event_; }

inline int Channel::happended() const { return happended_; }

inline void Channel::SetDeleted() {
  // Existed in map
  happended_ = Channel::IsDeleted;
}

inline void Channel::SetAdded() { happended_ = Channel::IsAdded; }

inline void Channel::TimeOutCallBack() {
  if (timeOutCallBack) timeOutCallBack();
}

inline void Channel::SetFd(int fd) { fd_ = fd; }

inline void Channel::SetEventLooper(EventLoop* looper) { looper_ = looper; }

}  // namespace pinkx
#endif
