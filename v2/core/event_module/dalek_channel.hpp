#pragma once

#include <sys/epoll.h>

#include <functional>

namespace core {

class event_loop;

class event_channel {
private:
    int fd_;
    event_loop *looper_;

    // call back
    std::function<void()> readCallBack;    // epoll in
    std::function<void()> writeCallBack;   // epoll out
    std::function<void()> errorCallBack;   // epollerr epollnvl
    std::function<void()> closeCallBack;   // epollrdhup epollhup

    int event_;     // Happending
    int revent_;    // User
    int happended_; // new or deleted or added
public:
    event_channel() = default;

    event_channel(const event_channel &) = delete;
    event_channel &operator=(const event_channel &) = delete;

    event_channel(event_channel &&c) :
        fd_(c.fd_),
        looper_(c.looper_),
        readCallBack(std::move(c.readCallBack)),
        writeCallBack(std::move(c.writeCallBack)),
        errorCallBack(std::move(c.errorCallBack)),
        closeCallBack(std::move(c.closeCallBack)),
        event_(c.event_),
        revent_(c.revent_),
        happended_(c.happended_) {
        c.fd_ = -1;
        c.looper_ = nullptr;
    }

    event_channel &operator=(event_channel &&c) {
        fd_ = c.fd_;
        looper_ = c.looper_;
        readCallBack = std::move(c.readCallBack);
        writeCallBack = std::move(c.writeCallBack);
        errorCallBack = std::move(c.errorCallBack);
        closeCallBack = std::move(c.closeCallBack);
        event_ = c.event_;
        revent_ = c.revent_;
        happended_ = c.happended_;

        c.fd_ = -1;
        c.looper_ = nullptr;
        return *this;
    }

    event_channel(event_loop &looper, int fd);
    ~event_channel() = default;

    void reset();

    void CallBack();

    void SetReadCallBack(std::function<void()> cb);
    void SetWriteCallBack(std::function<void()> cb);
    void SetCloseCallBack(std::function<void()> cb);
    void SetErrorCallBack(std::function<void()> cb);

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
    void SetEventLooper(event_loop &looper);

    event_loop *looper() const {
        return looper_;
    }
};

inline event_channel::event_channel(event_loop &looper, int fd) :
    looper_(&looper), fd_(fd), event_(0), revent_(0), happended_(0) {
}

inline void event_channel::CallBack() {
    if ((revent_ & (EPOLLHUP | EPOLLRDHUP) && (!revent_ & EPOLLIN))) {
        if (closeCallBack) closeCallBack();
        return;
    }

    if (revent_ & (EPOLLERR)) {
        if (errorCallBack) errorCallBack();
        return;
    }

    if (revent_ & (EPOLLIN | EPOLLPRI)) {
        if (readCallBack) readCallBack();
    }

    if (revent_ & (EPOLLOUT)) {
        if (writeCallBack) writeCallBack();
    }
}

inline void event_channel::SetReadCallBack(std::function<void()> cb) {
    readCallBack = std::move(cb);
}

inline void event_channel::SetWriteCallBack(std::function<void()> cb) {
    writeCallBack = std::move(cb);
}

inline void event_channel::SetErrorCallBack(std::function<void()> cb) {
    errorCallBack = std::move(cb);
}

inline void event_channel::SetCloseCallBack(std::function<void()> cb) {
    closeCallBack = std::move(cb);
}
 

inline void event_channel::SetRevent(int op) {
    revent_ = 0;
    revent_ |= op;
}

inline int event_channel::event() const {
    return event_;
}

inline int event_channel::happended() const {
    return happended_;
}

inline void event_channel::SetDeleted() {
    // Existed in map
    happended_ = event_channel::IsDeleted;
}

inline void event_channel::SetAdded() {
    happended_ = event_channel::IsAdded;
}

inline void event_channel::SetFd(int fd) {
    fd_ = fd;
}

inline void event_channel::SetEventLooper(event_loop &looper) {
    looper_ = &looper;
}

} // namespace core