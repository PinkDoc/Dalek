project(Dalek)

set(CMAKE_CXX_STANDARD 17)

SET(SRC_LISTS base/copyable.h base/noncopyable.h  base/Logger.h reactor/base.h  reactor/EventLoop.h reactor/Poller.h  reactor/Channel.h  reactor/InetAddress.h reactor/Socket.h reactor/TimerWheel.h http/base.h http/Buffer.h http/httppar.h http/httpres.h http/mime.h http/httpd.h Dalek.cc)

add_executable(Dalek ${SRC_LISTS})
