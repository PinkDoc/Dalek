#pragma once

#include "dalek_channel.hpp"
#include "dalek_poller.hpp"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cassert>
#include <vector>
#include <ctime>
#include <csignal>

namespace core {

class poller;

class event_loop {
private:
    int pollTimeMs_;
    std::vector<event_channel *> activeChannels_;
    bool quit_;

    poller poller_;
public:
    event_loop();

    // Noncopyable ==
    event_loop(const event_loop &) = delete;
    event_loop &operator=(const event_loop &) = delete;

    event_loop(event_loop &&l) :
        pollTimeMs_(l.pollTimeMs_),
        activeChannels_(std::move(activeChannels_)),
        quit_(l.quit_),
        poller_(std::move(l.poller_)) {
        processLooper = this;
    }

    event_loop &operator=(event_loop &&l) {
        pollTimeMs_ = l.pollTimeMs_;
        activeChannels_ = std::move(l.activeChannels_);
        quit_ = l.quit_;
        poller_ = std::move(l.poller_);

        processLooper = this;

        return *this;
    }

    void loop();

    void update(event_channel &c);
    void update(event_channel &c, std::time_t timeOut);
    void remove(event_channel &c);
    int poll_time() const;
    void SetPollTime(int timeOut);
    void stop();

    static event_loop *processLooper;
};

event_loop *event_loop::processLooper = nullptr;

inline event_loop::event_loop() :
    quit_(false), poller_(*this), pollTimeMs_(-1) {
    if (processLooper != nullptr) {
        // TODO do log
        assert(false);
    }

    signal(SIGPIPE, SIG_IGN);

    processLooper = this;
}

inline void event_loop::loop() {
    while (!quit_) {
        // poll
        activeChannels_.clear();
        poller_.poll(pollTimeMs_, activeChannels_);
        for (auto &i : activeChannels_) {
            i->CallBack();
        }
    }
}

inline void event_loop::update(event_channel &c) {
    poller_.update(&c);
}

inline void event_loop::remove(event_channel &c) {
    poller_.remove(&c);
}

inline void event_loop::SetPollTime(int ms) {
    pollTimeMs_ = ms;
}

inline int event_loop::poll_time() const {
    return pollTimeMs_;
}

inline void event_loop::stop() {
    quit_ = true;
}

inline void event_channel::EnableRead() {
    event_ |= (EPOLLIN);
    looper_->update(*this);
}

inline void event_channel::EnableWrite() {
    event_ |= EPOLLOUT;
    looper_->update(*this);
}

inline void event_channel::DisableRead() {
    event_ &= ~EPOLLIN;
    looper_->update(*this);
}

inline void event_channel::DisableWrite() {
    event_ &= ~EPOLLOUT;
    looper_->update(*this);
}

inline void event_channel::reset() {
    event_ = 0;
    revent_ = 0;
    happended_ = 0;
    looper_->remove(*this);
}

} // namespace core
