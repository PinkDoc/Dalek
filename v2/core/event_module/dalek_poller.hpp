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

class event_loop;

class poller {
private:
    int epollFd_;

    std::map<int, event_channel *> channels_;
    std::array<epoll_event, 16384> events_; // 4096 * 4

    event_loop *looper_;

    void fill(int num, std::vector<event_channel *> &list);
    int update(event_channel *ch, int op);

public:
    poller(event_loop &looper);
    ~poller();

    // Noncopyable
    poller(const poller &) = delete;
    poller &operator=(const poller &) = delete;

    inline poller(poller &&p) :
        epollFd_(p.epollFd_),
        channels_(std::move(p.channels_)),
        events_(std::move(p.events_)),
        looper_(p.looper_) {
        p.epollFd_ = -1;
        p.looper_ = nullptr;
    }

    inline poller &operator=(poller &&p) {
        epollFd_ = p.epollFd_;
        channels_ = std::move(p.channels_);
        events_ = (std::move(p.events_));
        looper_ = p.looper_;

        p.epollFd_ = -1;
        p.looper_ = nullptr;
        return *this;
    }

    void poll(int timeOut, std::vector<event_channel *> &list);

    // Add or update or delete
    int update(event_channel *ch);
    void remove(event_channel *ch);
};

inline poller::poller(event_loop &looper) :
    looper_(&looper), epollFd_(::epoll_create1(EPOLL_CLOEXEC)) {
    if (epollFd_ == -1) {
        assert(false);
    }
}

inline poller::~poller() {
    ::close(epollFd_);
}

inline int poller::update(event_channel *ch) {
    int fd = ch->fd();
    if (ch->happended() == event_channel::IsNew || ch->happended() == event_channel::IsDeleted) {
        channels_[fd] = ch;
        return update(ch, EPOLL_CTL_ADD);
    } else {
        if (channels_[fd] != ch) {
            assert(false);
        }
        return update(ch, EPOLL_CTL_MOD);
    }
}

inline int poller::update(event_channel *ch, int op) {
    epoll_event event;
    ::bzero(&event, sizeof event);
    event.data.fd = ch->fd();
    event.data.ptr = ch;
    // Register
    event.events = ch->event();

    int ret = epoll_ctl(epollFd_, op, ch->fd(), &event);

    if (ret == -1) {
        return -1;
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
    return ret;
}

inline void poller::fill(int num, std::vector<event_channel *> &list) {
    for (size_t i = 0; i < num; ++i) {
        int fd = events_[i].data.fd;
        event_channel *ch = static_cast<event_channel *>(events_[i].data.ptr);
        ch->SetRevent(events_[i].events);
        list.push_back(ch);
    }
}

inline void poller::poll(int timeOut, std::vector<event_channel *> &list) {
    int number = epoll_wait(epollFd_, events_.data(), events_.size(), timeOut);
    if (number > 0) {
        fill(number, list);
    }
}

inline void poller::remove(event_channel *ch) {
    int fd = ch->fd();
    if (channels_.find(fd) != channels_.end()) {
        channels_.erase(fd);
    }
    update(ch, EPOLL_CTL_DEL);
}

} // namespace core
