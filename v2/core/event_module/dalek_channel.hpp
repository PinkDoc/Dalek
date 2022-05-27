#pragma once

#include "dalek_eventloop.hpp"

#include <sys/epoll.h>

#include <functional>

namespace core {

class EventLoop;

class Channel {
private:
    int fd_;
    EventLoop *looper_;

    // call back
    std::function<void()> readCallBack;    // epoll in
    std::function<void()> writeCallBack;   // epoll out
    std::function<void()> errorCallBack;   // epollerr epollnvl
    std::function<void()> timeOutCallBack; // time out
    std::function<void()> closeCallBack;   // epollrdhup epollhup

    int event_;     // Happending
    int revent_;    // User
    int happended_; // new or deleted or added
public:
    Channel() = default;

    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;

    Channel(Channel &&c) :
        fd_(c.fd_),
        looper_(c.looper_),
        readCallBack(std::move(c.readCallBack)),
        writeCallBack(std::move(c.writeCallBack)),
        errorCallBack(std::move(c.errorCallBack)),
        timeOutCallBack(std::move(c.timeOutCallBack)),
        closeCallBack(std::move(c.closeCallBack)),
        event_(c.event_),
        revent_(c.revent_),
        happended_(c.happended_) {
        c.fd_ = -1;
        c.looper_ = nullptr;
    }

    Channel &operator=(Channel &&c) {
        fd_ = c.fd_;
        looper_ = c.looper_;
        readCallBack = std::move(c.readCallBack);
        writeCallBack = std::move(c.writeCallBack);
        errorCallBack = std::move(c.errorCallBack);
        timeOutCallBack = std::move(c.timeOutCallBack);
        closeCallBack = std::move(c.closeCallBack);
        event_ = c.event_;
        revent_ = c.revent_;
        happended_ = c.happended_;

        c.fd_ = -1;
        c.looper_ = nullptr;
        return *this;
    }

    Channel(EventLoop &looper, int fd);
    ~Channel() = default;

    void reset();

    void CallBack();

    void SetReadCallBack(std::function<void()> cb);
    void SetWriteCallBack(std::function<void()> cb);
    void SetCloseCallBack(std::function<void()> cb);
    void SetErrorCallBack(std::function<void()> cb);
    void SetTimeOutCallBack(std::function<void()> cb);

    inline int fd() const {
        return fd_;
    }

    static constexpr int IsNew = 0;
    static constexpr int IsDeleted = -1;
    static constexpr int IsAdded = 1;

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
    void SetEventLooper(EventLoop &looper);

    EventLoop *looper() const {
        return looper_;
    }
};

inline Channel::Channel(EventLoop &looper, int fd) :
    looper_(&looper), fd_(fd), event_(0), revent_(0), happended_(0) {
}

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

inline void Channel::SetReadCallBack(std::function<void()> cb) {
    readCallBack = std::move(cb);
}

inline void Channel::SetWriteCallBack(std::function<void()> cb) {
    writeCallBack = std::move(cb);
}

inline void Channel::SetErrorCallBack(std::function<void()> cb) {
    errorCallBack = std::move(cb);
}

inline void Channel::SetCloseCallBack(std::function<void()> cb) {
    closeCallBack = std::move(cb);
}

inline void Channel::SetTimeOutCallBack(std::function<void()> cb) {
    timeOutCallBack = std::move(cb);
}

inline void Channel::SetRevent(int op) {
    revent_ = 0;
    revent_ |= op;
}

inline int Channel::event() const {
    return event_;
}

inline int Channel::happended() const {
    return happended_;
}

inline void Channel::SetDeleted() {
    // Existed in map
    happended_ = Channel::IsDeleted;
}

inline void Channel::SetAdded() {
    happended_ = Channel::IsAdded;
}

inline void Channel::TimeOutCallBack() {
    if (timeOutCallBack) timeOutCallBack();
}

inline void Channel::SetFd(int fd) {
    fd_ = fd;
}

inline void Channel::SetEventLooper(EventLoop &looper) {
    looper_ = &looper;
}

} // namespace core