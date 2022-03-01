// httpd
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <unordered_map>

#include "buffer.hpp"
#include "base.hpp"
#include "httppar.hpp"
#include "httpres.hpp"

namespace pinkx {
namespace http {
//
//
//
#define CONNECTION_TIME_OUT 8
#define CRLF "\r\n"
#define SERVER_STRING "Dalek"
#define HTTP_VERSION "HTTP/1.1 "

class HttpConnection : noncopyable {
 private:
  enum handle_state {
    READ_NOT_READY = 0,
    READ_READY,
    WRITE_NOT_READY,
    WRITE_READY
  };

  int fd_;
  Channel channel_;
  EventLoop* looper_;
  TimerWheel* timer_wheel_;

  HttpParser parser_;
  HttpRequest request_;

  Buffer readBuffer_;
  Buffer writeBuffer_;

  int handle_state_;

  int file_fd_;

  std::string_view GetMime();

  // void ExceCgi();   TODO
  // Build error and delete timer and close fd_
  void BuildError(rescode code);
  void CloseConnection();

 public:
  HttpConnection(EventLoop& looper, TimerWheel& wheel, int fd)
      : fd_(fd),
        channel_(looper, fd),
        looper_(&looper),
        timer_wheel_(&wheel),
        request_(),
        parser_(request_),
        handle_state_(READ_NOT_READY),
        file_fd_(-1)

  {
    request_.packet_ = readBuffer_.peek();
    request_.len_ = 0;

    timer_wheel_->insert(channel_, CONNECTION_TIME_OUT);

    channel_.EnableRead();

    channel_.SetTimeOutCallBack(
        std::bind(&HttpConnection::CloseConnection, this));
    channel_.SetReadCallBack(
            std::bind(&HttpConnection::PollIn, this));
    channel_.SetWriteCallBack(
            std::bind(&HttpConnection::PollOut, this));
    channel_.SetErrorCallBack(
            std::bind(&HttpConnection::CloseConnection, this));

    looper_->update(channel_);
  }

  ~HttpConnection() { CloseConnection(); }

  void PollIn();

  void HandleResponse();  // Parse and do other...

  void PollOut();
  void SendFile();

  void reset();

  void Register();  // Register connect to looper and wheel
};

void HttpConnection::PollIn() {
  int ret = readBuffer_.readFromFd(fd_);
  if (request_.packet_ != readBuffer_.peek()) {
    // resize
    request_.packet_ = readBuffer_.peek();
  }
  request_.len_ = readBuffer_.size();
  errcode code = parser_.parse_HTTP();

  // bad request
  if (code == ERROR_REQUEST || code == ERROR_URI || code == ERROR_HEAD ||
      code == ERROR_EMPTY_REQUEST) {
    BuildError(BAD_REQUEST);
    return;
  }

  if (code == PARSE_EMPTY_LINE) {
    HandleResponse();
  }
}

void HttpConnection::HandleResponse() {
  auto method = request_.method_;
  if (request_.version_ != 11) {
    BuildError(VERSION_NOT_SUPPORT);
    return;
  }
  if (method == HTTP_GET) {
    HttpURI& uri = request_.uri_;
    if (uri.query.size() > 0) {
      request_.cgi == true;
    }
    if (uri.path == "/" || uri.path == "") uri.path = "index.html";

    stat(uri.path.c_str(), &request_.fileStat_);

    file_fd_ = open(uri.path.c_str(), O_NONBLOCK | O_RDONLY);

    if (file_fd_ == -1)
    {
      if (uri.path == "index.html")
        BuildError(FORBIDDEN);
      else
        BuildError(NOT_FOUND);
      return;
    } else {
      request_.rescode_ = OK;
    }

  }
  else if (method == HTTP_POST)
  {
    request_.cgi = true;
    request_.rescode_ = OK;
  }
  else if (method == HTTP_HEAD)
  {
      HttpURI& uri = request_.uri_;

      if (uri.path == "/" || uri.path == "") uri.path = "index.html";

      int ret = stat(uri.path.c_str(), &request_.fileStat_);

      if (ret == -1) {
          if (uri.path == "index.html")
              BuildError(FORBIDDEN);
          else
              BuildError(NOT_FOUND);
          return;
      } else {
          request_.rescode_ = OK;
      }
  }
  else if (method == HTTP_TRACE)
  {
      request_.rescode_ = OK;
      request_.fileStat_.st_size = sizeof(readBuffer_.size());
  }
  else
  {
    BuildError(NOT_IMPLEMENT);
    return;
  }

  auto& head = request_.head_.GetHeads();
  for (auto& i : head) {
    if (i.first == "Connection") {
      if (i.second == "keep-alive") request_.keep_alive_ = true;
    }
  }

  writeBuffer_.sprintf(HTTP_VERSION);
  writeBuffer_.sprintf(const_cast<char*>(resMap[request_.rescode_].data()));
  writeBuffer_.sprintf(CRLF);
  writeBuffer_.sprintf("Server: ");
  writeBuffer_.sprintf(SERVER_STRING CRLF);
  if (request_.keep_alive_ == true) {
    writeBuffer_.sprintf("Connection: %s" CRLF, "keep-alive");
  } else {
    writeBuffer_.sprintf("Connection: %s" CRLF, "close");
  }
  writeBuffer_.sprintf("Content-Type: %s" CRLF, GetMime().data());
  writeBuffer_.sprintf("Content-Length: %d" CRLF, request_.fileStat_.st_size);
  writeBuffer_.sprintf(CRLF);

  if (request_.method_ == HTTP_TRACE)
  {
      writeBuffer_.sprintf(readBuffer_.peek());
  }

  channel_.DisableRead();
  channel_.EnableWrite();
  looper_->update(channel_);
}

void HttpConnection::BuildError(rescode code) {
  writeBuffer_.sprintf(HTTP_VERSION);
  writeBuffer_.sprintf(const_cast<char*>(resMap[request_.rescode_].data()));
  writeBuffer_.sprintf(CRLF);
  writeBuffer_.sprintf("Server: ");
  writeBuffer_.sprintf(SERVER_STRING CRLF);
  std::string_view* packet;

  if (code == NOT_FOUND) {
    packet = &err_404_page;
  } else if (code == BAD_REQUEST) {
    packet = &err_400_page;
  } else if (code == VERSION_NOT_SUPPORT) {
    packet = &err_505_page;
  } else if (code == FORBIDDEN) {
    packet = &err_403_page;
  } else if (code == NOT_IMPLEMENT) {
    packet = &err_501_page;
  }

  writeBuffer_.sprintf("Content-Type: text/html" CRLF);
  writeBuffer_.sprintf("Content-Length: %d" CRLF CRLF,
                       packet->size() + err_page_tail.size());
  writeBuffer_.sprintf(const_cast<char*>(packet->data()));
  writeBuffer_.sprintf(const_cast<char*>(err_page_tail.data()));
  request_.keep_alive_ = false;
  timer_wheel_->del(channel_);
  channel_.DisableRead();
  channel_.EnableWrite();
  looper_->update(channel_);
}

void HttpConnection::PollOut() {
  int ret = writeBuffer_.sendToFd(fd_);
  if (writeBuffer_.writeable() != writeBuffer_.capacity()) return;
  SendFile();
  // All send out
  if (request_.keep_alive_ == true) {
    timer_wheel_->insert(channel_, CONNECTION_TIME_OUT);
    reset();
    looper_->update(channel_);
  } else {
    CloseConnection();
  }
}

void HttpConnection::CloseConnection() {
  looper_->remove(channel_);
  timer_wheel_->del(channel_);
  close(fd_);
  reset();
}

void HttpConnection::SendFile() {
  if (file_fd_ == -1) return;
  int ret = 0;
  while (true) {
    int ret = sendfile(fd_, file_fd_, nullptr, 1);
    if (ret == 0 || ret < 0) break;
  }
  close(file_fd_);
  file_fd_ = -1;
  // if (ret < 0) CloseConnection();
}

std::string_view HttpConnection::GetMime() {
  auto& path = request_.uri_.path;
  size_t dot = path.find_last_of('.');
  if (dot == path.npos) {
    return "text/plain";
  }
  auto suffix = path.substr(dot);
  auto ret = mimeMap.find(suffix);
  if (ret == mimeMap.end()) {
    return "text/plain";
  }

  return (*ret).second;
}

void HttpConnection::reset() {
  request_.reset();
  parser_.reset();

  handle_state_ = READ_NOT_READY;

  readBuffer_.reset();
  writeBuffer_.reset();

  channel_.EnableRead();
  channel_.DisableWrite();
}

void HttpConnection::Register() {
  timer_wheel_->insert(channel_, CONNECTION_TIME_OUT);
  looper_->update(channel_);
}

//
//
//
//  Reuse port and Master / Worker
//
//
//
class HttpServer : noncopyable {
 private:
  TimerWheel* timer_wheel_;  // Register connection to timer
  EventLoop* looper_;        // Register connection to looper

  Channel channel_;

  int fd_;
  net::Socket socket_;
  net::InetAddress address_;
  std::unordered_map<int, HttpConnection*> connections_;

 public:
  // Default port is 8000
  HttpServer(EventLoop& looper, TimerWheel& wheel)
      : looper_(&looper),
        timer_wheel_(&wheel),
        fd_(::socket(AF_INET, SOCK_STREAM, 0)),
        channel_(looper, fd_),
        address_(8000, false),
        socket_(fd_) {
    assert(fd_ > 0);
    socket_.SetReusePort(true);  // Port resuse
    socket_.UseNagle(false);     // Not use Nagle
    socket_.SetBindAddress(address_);
    socket_.listen();

    channel_.EnableRead();
    channel_.SetReadCallBack(std::bind(&HttpServer::accept, this));
    looper_->update(channel_);
  }

  HttpServer(EventLoop& looper, TimerWheel& wheel, net::InetAddress& address)
      : looper_(&looper),
        timer_wheel_(&wheel),
        fd_(::socket(AF_INET, SOCK_STREAM, 0)),
        channel_(looper, fd_),
        address_(address),
        socket_(fd_) {
    assert(fd_ > 0);
    channel_.SetFd(fd_);
    socket_.SetFd(fd_);
    net::SetNonBlocking(fd_);

    channel_.EnableRead();
    channel_.SetReadCallBack(std::bind(&HttpServer::accept, this));
    looper_->update(channel_);

    socket_.SetReusePort(true);  // Port resuse
    socket_.UseNagle(false);     // Not use Nagle
    socket_.SetBindAddress(address_);
    socket_.listen();
  }

  ~HttpServer() {
    close(fd_);
    for (auto i : connections_) delete i.second;
  }

  void accept();
};

void HttpServer::accept() {
  net::InetAddress addr_;
  socklen_t len = sizeof(sockaddr_in);
  int ret = ::accept4(fd_, addr_.GetAddr(), &len, O_NONBLOCK);
  if (ret > 0) {
    if (connections_.find(ret) == connections_.end()) {
      HttpConnection* new_client =
          new HttpConnection(*looper_, *timer_wheel_, ret);
      connections_.emplace(ret, new_client);
    } else {
      connections_[ret]->Register();
    }
  }
}

}  // namespace http
}  // namespace pinkx
#endif