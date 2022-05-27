#pragma once

#include "dalek_channel.hpp"
#include "dalek_eventloop.hpp"

#include <vector>
#include <map>

#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/epoll.h>

namespace core {

class EventLoop;

class Poller {
private:
    int epollFd_;

    std::map<int, Channel *> channels_;
    std::array<epoll_event, 16384> events_; // 4096 * 4

    EventLoop *looper_;

    void fill(int num, std::vector<Channel *> &list);
    void update(Channel *ch, int op);

public:
    Poller(EventLoop &looper);
    ~Poller();

    // Noncopyable
    Poller(const Poller &) = delete;
    Poller &operator=(const Poller &) = delete;

    Poller(Poller &&p) :
        epollFd_(p.epollFd_),
        channels_(std::move(p.channels_)),
        events_(std::move(p.events_)),
        looper_(p.looper_) {
        p.epollFd_ = -1;
        p.looper_ = nullptr;
    }

    Poller &operator=(Poller &&p) {
        epollFd_ = p.epollFd_;
        channels_ = std::move(p.channels_);
        events_ = (std::move(p.events_));
        looper_ = p.looper_;

        p.epollFd_ = -1;
        p.looper_ = nullptr;
        return *this;
    }

    void poll(int timeOut, std::vector<Channel *> &list);

    // Add or update or delete
    void update(Channel *ch);
    void remove(Channel *ch);
};

inline Poller::Poller(EventLoop &looper) :
    looper_(&looper), epollFd_(::epoll_create1(EPOLL_CLOEXEC)) {
    if (epollFd_ == -1) {
        assert(false);
    }
}

inline Poller::~Poller() {
    ::close(epollFd_);
}

inline void Poller::update(Channel *ch) {
    int fd = ch->fd();
    if (ch->happended() == Channel::IsNew || ch->happended() == Channel::IsDeleted) {
        channels_[fd] = ch;
        update(ch, EPOLL_CTL_ADD);
    } else {
        if (channels_[fd] != ch) {
            assert(false);
        }
        update(ch, EPOLL_CTL_MOD);
    }
}

inline void Poller::update(Channel *ch, int op) {
    epoll_event event;
    ::bzero(&event, sizeof event);
    event.data.fd = ch->fd();
    event.data.ptr = ch;
    // Register
    event.events = ch->event();

    int ret = epoll_ctl(epollFd_, op, ch->fd(), &event);

    if (ret == -1) {
        return;
    }

    if (op == EPOLL_CTL_MOD) {
        // Update
    } else if (op == EPOLL_CTL_DEL) {
        ch->SetDeleted();
        ch->DisableWrite();
        ch->DisableRead();
    } else {
        ch->SetAdded();
    }
}

inline void Poller::fill(int num, std::vector<Channel *> &list) {
    for (size_t i = 0; i < num; ++i) {
        int fd = events_[i].data.fd;
        Channel *ch = static_cast<Channel *>(events_[i].data.ptr);
        ch->SetRevent(events_[i].events);
        list.push_back(ch);
    }
}

inline void Poller::poll(int timeOut, std::vector<Channel *> &list) {
    int number = epoll_wait(epollFd_, events_.data(), events_.size(), timeOut);
    if (number > 0) {
        fill(number, list);
    }
}

inline void Poller::remove(Channel *ch) {
    int fd = ch->fd();
    if (channels_.find(fd) != channels_.end()) {
        channels_.erase(fd);
    }
    update(ch, EPOLL_CTL_DEL);
}

} // namespace core
