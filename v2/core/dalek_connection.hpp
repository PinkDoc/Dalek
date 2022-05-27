#pragma once

#include "dalek_tcp.hpp"
#include "dalek_buffer.hpp"
#include "event_module/dalek_eventloop.hpp"

#include <vector>
#include <functional>

namespace core {
class listener;
class connection;

using MessageCallBack = std::function<void(connection &)>;
using SendCallBack = std::function<void(connection &)>;
using ErrorCallBack = std::function<void(connection &)>;
using CloseCallBack = std::function<void(connection &)>;
using TimeOutCallBack = std::function<void(connection &)>;
using AcceptCallBack = std::function<void(listener &, connection &)>;

class connection {
public:
    enum state {
        Connecting,
        DisConnecting,
        NonConnecting
    };

private:
    Channel chan_;
    Buffer rdBuffer_;
    Buffer wrBuffer_;

    tcp::tcpsocket socket_;

    Timer *timer_;

    MessageCallBack msgCb_;
    SendCallBack sendCb_;
    ErrorCallBack errCb_;
    CloseCallBack closeCb_;
    TimeOutCallBack timeoutCb_;

    state state_;

    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();
    void handleTimeOut();

    void readMessage() {
        int ret = rdBuffer_.readFromFd(socket_.Fd());
        if (ret == 0) state_ = DisConnecting;
    }
    void writeMessage() {
        int ret = wrBuffer_.sendToFd(socket_.Fd());
        state_ = DisConnecting;
    }

    // Close this connection
    void Close() {
        chan_.reset();
        rdBuffer_.reset();
        wrBuffer_.reset();
        state_ = NonConnecting;
        if (timer_)
            timer_->DelChannel(chan_);
        socket_.Close();
    }

    void reuse() {
        state_ = Connecting;
    }

public:
    constexpr static std::time_t DefaultTimeOut = 32;
    static std::time_t TimeOut;

    connection(EventLoop &looper, tcp::tcpsocket cs);
    connection(EventLoop &looper, int fd);
    connection(EventLoop &looper, tcp::tcpsocket cs, Timer &timer);

    int fd() {
        return chan_.fd();
    }

    void SetMsgCallBack(MessageCallBack cb) {
        msgCb_ = std::move(cb);
    }
    void SetSendCallBack(SendCallBack cb) {
        sendCb_ = std::move(cb);
    }
    void SetErrorCallBack(ErrorCallBack cb) {
        errCb_ = std::move(cb);
    }
    void SetCloseCallBack(CloseCallBack cb) {
        closeCb_ = std::move(cb);
    }
    void SetTimeOutCallBack(TimeOutCallBack cb) {
        timeoutCb_ = std::move(cb);
    }

    // epoll_ctl | EPOLLIN
    void EnableRead() {
        reuse();
        chan_.EnableRead();
    }
    // epoll_ctl & ~EPOLLIN
    void DisableRead() {
        reuse();
        chan_.DisableRead();
    }
    // epoll_ctl | EPOLLOUT
    void EnableWrite() {
        reuse();
        chan_.EnableWrite();
    }
    // epoll_ctl & ~EPOLLOUT
    void DisableWrite() {
        reuse();
        chan_.DisableWrite();
    }

    void ReloadTimer(std::time_t t) {
        timer_->insert(t, chan_);
    }
    void ReloadTimer() {
        timer_->insert(connection::TimeOut, chan_);
    }
    void DisableTimer() {
        timer_->DelChannel(chan_);
    }

    Buffer &readBuffer() {
        return rdBuffer_;
    }
    Buffer &writeBuffer() {
        return wrBuffer_;
    }
};

std::time_t connection::TimeOut = connection::DefaultTimeOut;

class listener {
private:
    Channel chan_;
    tcp::tcpsocket socket_;
    AcceptCallBack acceptCb_;
    Timer *timer_;

    std::map<int, connection *> connection_map_;

    void handleAccept();

public:
    listener(EventLoop &looper, ip::address_v4 a);
    listener(EventLoop &looper, ip::address_v4 a, Timer &t);

    void SetAcceptCallBack(AcceptCallBack cb) {
        acceptCb_ = std::move(cb);
    }
    connection *GetConnectionByFd(int fd) {
        return connection_map_.find(fd) == connection_map_.end() ? nullptr : connection_map_.at(fd);
    }
};

listener::listener(EventLoop &looper, ip::address_v4 a) :
    socket_(a),
    timer_(nullptr) {
    socket_.SetNonBlock();

    socket_.Bind();
    socket_.Listen();

    chan_.SetFd(socket_.Fd());
    chan_.SetEventLooper(looper);

    chan_.SetReadCallBack([this] {
        handleAccept();
    });

    chan_.EnableRead();
}

listener::listener(EventLoop &looper, ip::address_v4 a, Timer &t) :
    socket_(a),
    timer_(&t) {
    socket_.SetNonBlock();

    socket_.Bind();
    socket_.Listen();

    chan_.SetFd(socket_.Fd());
    chan_.SetEventLooper(looper);

    chan_.SetReadCallBack([this] {
        handleAccept();
    });

    chan_.EnableRead();
}

void listener::handleAccept() {
    auto cs = socket_.Accept();

    connection *c{nullptr};

    if (connection_map_.find(cs.Fd()) == connection_map_.end()) {
        if (!timer_)
            c = new connection(*chan_.looper(), std::move(cs));
        else
            c = new connection(*chan_.looper(), std::move(cs), *timer_);
        connection_map_.emplace(cs.Fd(), c);
    } else {
        c = connection_map_.at(cs.Fd());
    }

    if (acceptCb_) {
        acceptCb_(*this, *c);
    }
}

connection::connection(EventLoop &looper, tcp::tcpsocket cs) :
    chan_(looper, cs.Fd()),
    socket_(std::move(cs)),
    state_(Connecting) {
    socket_.SetNonBlock();

    chan_.SetReadCallBack([this] {
        handleRead();
    });
    chan_.SetWriteCallBack([this] {
        handleWrite();
    });
    chan_.SetErrorCallBack([this] {
        handleClose();
    });
}

connection::connection(EventLoop &looper, int fd) :
    chan_(looper, fd),
    socket_(fd),
    state_(Connecting) {
    socket_.SetNonBlock();

    chan_.SetReadCallBack([this] {
        handleRead();
    });
    chan_.SetWriteCallBack([this] {
        handleWrite();
    });
    chan_.SetErrorCallBack([this] {
        handleClose();
    });
}

connection::connection(EventLoop &looper, tcp::tcpsocket cs, Timer &t) :
    chan_(looper, cs.Fd()),
    socket_(std::move(cs)),
    timer_(&t),
    state_(Connecting) {
    socket_.SetNonBlock();

    chan_.SetReadCallBack([this] {
        handleRead();
    });
    chan_.SetWriteCallBack([this] {
        handleWrite();
    });
    chan_.SetErrorCallBack([this] {
        handleClose();
    });
    chan_.SetTimeOutCallBack([this] {
        handleTimeOut();
    });

    timer_->AddChannel(TimeOut, chan_);
}

void connection::handleRead() {
    if (state_ == Connecting) {
        readMessage();

        if (state_ == DisConnecting) {
            handleClose();
        }

        if (msgCb_) {
            msgCb_(*this);
        }
    } else if (state_ == DisConnecting) {
        handleClose();
    } else {
        // TODO
    }
}

void connection::handleWrite() {
    if (state_ == Connecting) {
        writeMessage();

        if (state_ == DisConnecting) {
            handleClose();
        }

        if (msgCb_) {
            msgCb_(*this);
        }
    } else if (state_ == DisConnecting) {
        handleClose();
    } else {
        // TODO
    }
}

void connection::handleError() {
    if (errCb_) {
        errCb_(*this);
    }

    Close();
}

void connection::handleClose() {
    if (closeCb_) {
        closeCb_(*this);
    }

    Close();
}

void connection::handleTimeOut() {
    if (timeoutCb_) {
        timeoutCb_(*this);
    }

    Close();
}

} // namespace core