#pragma once

#include "dalek_channel.hpp"
#include "dalek_poller.hpp"

#include <sys/timerfd.h>
#include <unistd.h>

#include <vector>
#include <ctime>

namespace imple {
class List {
private:
    std::vector<core::Channel *> channels_;
    std::size_t max_;

public:
    List() = default;
    List(const List &) = delete;
    List &operator=(const List &) = delete;

    // This is a slot of timer wheel, and if timer wheel tick it will use the channel's timeoutcallback
    void tick() {
        for (auto i : channels_) {
            if (i)
                i->TimeOutCallBack();
        }
        channels_.clear();
    }

    std::size_t insert(core::Channel &c) {
        channels_.push_back(&c);
        return channels_.size() - 1;
    }

    void DelChannel(int p) {
        channels_[p] = nullptr;
    }
};

} // namespace imple

namespace core {

class EventLoop;
class Channel;
class Poller;

class Timer {
private:
    using List = imple::List;

    std::time_t nowTime_;

    core::EventLoop *looper_;

    int fd_;
    itimerspec howlong_;
    core::Channel chann_;
    List list_;

    std::vector<List> slots_;

    std::map<core::Channel *, std::pair<List *, int>> marker_;

    std::size_t clockhands_;

public:
    Timer(EventLoop &);

    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;

    ~Timer() {
        close(fd_);
    }

    void AddChannel(std::time_t timeOut, core::Channel &channel);
    void UpdateChannel(std::time_t timeOut, core::Channel &channel);
    void DelChannel(core::Channel &channel);
    void tick();

    // Please use insert
    void insert(std::time_t timeOut, core::Channel &channel);
};

class EventLoop {
private:
    int pollTimeMs_;
    std::vector<Channel *> activeChannels_;
    bool quit_;

    Poller poller_;

public:
    EventLoop();

    // Noncopyable ==
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    EventLoop(EventLoop &&l) :
        pollTimeMs_(l.pollTimeMs_),
        activeChannels_(std::move(activeChannels_)),
        quit_(l.quit_),
        poller_(std::move(l.poller_)) {
        processLooper = this;
    }

    EventLoop &operator=(EventLoop &&l) {
        pollTimeMs_ = l.pollTimeMs_;
        activeChannels_ = std::move(l.activeChannels_);
        quit_ = l.quit_;
        poller_ = std::move(l.poller_);

        processLooper = this;

        return *this;
    }

    void loop();

    void update(Channel &c);
    void update(Channel &c, std::time_t timeOut);
    void remove(Channel &c);
    int poll_time() const;
    void SetPollTime(int timeOut);
    void stop();

    static EventLoop *processLooper;
};

EventLoop *EventLoop::processLooper = nullptr;

inline EventLoop::EventLoop() :
    quit_(false), poller_(*this), pollTimeMs_(-1) {
    if (processLooper != nullptr) {
        // TODO do log
        assert(false);
    }
    processLooper = this;
}

inline void EventLoop::loop() {
    while (!quit_) {
        // poll
        activeChannels_.clear();
        poller_.poll(pollTimeMs_, activeChannels_);
        for (auto &i : activeChannels_) {
            i->CallBack();
        }
    }
}

inline void EventLoop::update(Channel &c) {
    poller_.update(&c);
}

inline void EventLoop::remove(Channel &c) {
    poller_.remove(&c);
}

inline void EventLoop::SetPollTime(int ms) {
    pollTimeMs_ = ms;
}

inline int EventLoop::poll_time() const {
    return pollTimeMs_;
}

inline void EventLoop::stop() {
    quit_ = true;
}

// FXXX c++
inline void Channel::EnableRead() {
    event_ |= (EPOLLIN);
    looper_->update(*this);
}

inline void Channel::EnableWrite() {
    event_ |= EPOLLOUT;
    looper_->update(*this);
}

inline void Channel::DisableRead() {
    event_ &= ~EPOLLIN;
    looper_->update(*this);
}

inline void Channel::DisableWrite() {
    event_ &= ~EPOLLOUT;
    looper_->update(*this);
}

Timer::Timer(core::EventLoop &looper) :
    nowTime_(std::time(nullptr)),
    fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
    looper_(&looper),
    slots_(256), // 256 seconds
    clockhands_(0) {
    bzero(&howlong_, sizeof howlong_);
    howlong_.it_value.tv_sec = 1;
    timerfd_settime(fd_, 0, &howlong_, nullptr);

    chann_.SetFd(fd_);
    chann_.SetEventLooper(looper);
    chann_.SetReadCallBack(
        [this] {
            this->tick();
        });
    chann_.EnableRead();
}

inline void Timer::AddChannel(std::time_t timeOut, core::Channel &channel) {
    auto slotlen = slots_.size();
    auto p = (clockhands_ + timeOut) % slotlen;
    auto sp = slots_[p].insert(channel);
    marker_.insert({&channel, {(slots_.data() + p), sp}});
}

inline void Timer::UpdateChannel(std::time_t timeOut, core::Channel &channel) {
    auto slt = marker_[&channel].first;
    auto p = marker_[&channel].second;

    slt->DelChannel(p);

    auto slotlen = slots_.size();
    p = (clockhands_ + timeOut) % slotlen;
    auto sp = slots_[p].insert(channel);
    marker_[&channel].first = slots_.data() + p;
    marker_[&channel].second = sp;
}

inline void Timer::DelChannel(core::Channel &channel) {
    auto iter = marker_.find(&channel);
    if (iter != marker_.end()) {
        auto item = iter->second;
        auto s = item.first;
        auto p = item.second;
        s->DelChannel(p);
        marker_.erase(&channel);
    }
}

inline void Timer::tick() {
    slots_[clockhands_].tick();
    clockhands_ = (clockhands_ + 1) % slots_.size();

    char temp[1024] = {0};
    read(fd_, temp, 1024);
    timerfd_settime(fd_, 0, &howlong_, nullptr);
}

inline void Timer::insert(std::time_t timeOut, core::Channel &channel) {
    if (marker_.find(&channel) == marker_.end()) {
        AddChannel(timeOut, channel);
    } else {
        UpdateChannel(timeOut, channel);
    }
}

inline void Channel::reset() {
    event_ = 0;
    revent_ = 0;
    happended_ = 0;
    looper_->remove(*this);
}

} // namespace core
