// Poller
#ifndef PINK_POLL_H
#define PINK_POLL_H

#include <assert.h>
#include <string.h>
#include <sys/epoll.h>

#include <unordered_map>

#include "EventLoop.h"
#include "base.h"

namespace pinkx {

class EventLoop;

// All epoll event on stack
class Poller : noncopyable {
 private:
  int epollFd_;

  std::unordered_map<int, Channel *> channels_;
  std::array<epoll_event, 65535> events_;

  EventLoop *looper_;

  void fill(int num, std::vector<Channel *> &list);
  void update(Channel *ch, int op);

 public:
  Poller(EventLoop *looper);
  ~Poller();

  void poll(int timeOut, std::vector<Channel *> &list);

  // Add or update or delete
  void update(Channel *ch);
  void remove(Channel *ch);
};

Poller::Poller(EventLoop *looper)
    : looper_(looper), epollFd_(::epoll_create1(EPOLL_CLOEXEC)) {
  if (epollFd_ == -1) {
    P_LOG_ERROR("Poller::Poller()");
    assert(false);
  }

  P_LOG_TRACE("Poller::Poller()");
}

Poller::~Poller() {
  P_LOG_TRACE("Poller::~Poller()");
  close(epollFd_);
}

void Poller::update(Channel *ch) {
  int fd = ch->fd();
  if (ch->happended() == Channel::IsNew ||
      ch->happended() == Channel::IsDeleted) {
    channels_[fd] = ch;
    update(ch, EPOLL_CTL_ADD);
    P_LOG_TRACE("update(%d)", fd);
  } else {
    if (channels_[fd] != ch) {
      P_LOG_ERROR("update(%d)", fd);
      assert(false);
    }
    update(ch, EPOLL_CTL_MOD);
  }
}

void Poller::update(Channel *ch, int op) {
  epoll_event event;
  ::bzero(&event, sizeof event);
  event.data.fd = ch->fd();
  event.data.ptr = ch;
  // Register
  event.events = ch->event();

  int ret = epoll_ctl(epollFd_, op, ch->fd(), &event);

  if (ret == -1) {
    P_LOG_WARN("update(%d) fail!", ch->fd());
    return;
  }

  if (op == EPOLL_CTL_MOD) {
    P_LOG_TRACE("Poller::update() update(%d)", ch->fd());
  } else if (op == EPOLL_CTL_DEL) {
    ch->SetDeleted();
    ch->DisableWrite();
    ch->DisableRead();

    P_LOG_TRACE("Poller::update() delete(%d)", ch->fd());

  } else {
    ch->SetAdded();
    P_LOG_TRACE("Poller::update() added(%d)", ch->fd());
  }
}

void Poller::fill(int num, std::vector<Channel *> &list) {
  for (size_t i = 0; i < num; ++i) {
    int fd = events_[i].data.fd;
    Channel *ch = static_cast<Channel *>(events_[i].data.ptr);
    ch->SetRevent(events_[i].events);
    list.push_back(ch);
  }
}

void Poller::poll(int timeOut, std::vector<Channel *> &list) {
  int number = epoll_wait(epollFd_, events_.data(), events_.size(), timeOut);
  if (number > 0) {
    fill(number, list);
  }
}

void Poller::remove(Channel *ch) {
  int fd = ch->fd();
  if (channels_.find(fd) != channels_.end()) {
    channels_.erase(fd);
  }
  update(ch, EPOLL_CTL_DEL);
}



}  // namespace pinkx
#endif