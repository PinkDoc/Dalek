<a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>![Build Status](https://img.shields.io/badge/language-c++-writek.svg)
![dw](tadis.jpg)

Dalek跑着我的小站：
[Dalek](http://www.pinkdoc.cn)
网站稳定运行中......
## 搭建-Build
`chmod 777 -c build.sh`
`./build.sh`
## 用法-Usage
`./Dalek start [-p port] [-n worker_num]`

## 测压-Test
测试机器处理器：`i5-9300` ，系统 `Ubuntu 20.0.4`
10K条的长连接 ，维持时间4s, 4个worker
```
webbench -c 10000 -t 4 http://localhost:1989/
```
结果：
```
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://localhost:1989/
10000 clients, running 4 sec.

Speed=4841789 pages/min, 18630496 bytes/sec.
Requests: 322786 susceed, 0 failed.

```
## Dalek架构
`Master/Worker`模型，端口复用，内核实现了负载均衡，使用时间轮管理长连接 ，每个工作进程（worker）创建一个`EventLoop`和`HttpServer`，内部是`reactor`模型，`HttpServer`负责接受连接，注册连接进入`EventLoop`和`TimerWheel`。

## Dalek细节
`master`进程主要负责杀死`worker`进程， 当`worker`进程挂了，就会唤醒`master`进程，再`fork()`一个`worker`
`httppar.h`实现了一个高性能的http解析器，可以在客户端40ms发送1byte的情况下正确的解析http请求包 ，`reactor`目录下则对事件进行了抽象，任何IO事件都封装成`Channel`，包括定时事件`TimerWheel`，服务器事件`HttpServer`，连接事件`HttpConnection`, `Poller`则是封装了`epoll`，Dalek只使用了水平模式（LT）。

## 代码统计
* 语言

| language | files | code | comment | blank | total | comment rate |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: |
| C++ | 18 | 2,374 | 262 | 703 | 3,339 | 9.94% |
| Markdown | 1 | 82 | 0 | 17 | 99 | 0.00% |
| Shell Script | 1 | 3 | 1 | 2 | 6 | 25.00% |
* 目录

| path | files | code | comment | blank | total | comment rate |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: |
| . | 20 | 2,459 | 263 | 722 | 3,444 | 9.66% |
| base | 4 | 174 | 15 | 63 | 252 | 7.94% |
| http | 6 | 1,511 | 226 | 343 | 2,080 | 13.01% |
| reactor | 7 | 630 | 21 | 276 | 927 | 3.23% |

## 代码树

```
Dalek
├─ CMakeLists.txt
├─ Dalek.cc
├─ README.md
├─ base
│  ├─ SyncLogger.h
│  ├─ copyable.h
│  ├─ noncopyable.h
│  └─ swap.h
├─ build
├─ build.sh
├─ http
│  ├─ Buffer.h
│  ├─ base.h
│  ├─ httpd.h
│  ├─ httppar.h
│  ├─ httpres.h
│  └─ mime.h
└─ reactor
   ├─ Channel.h
   ├─ EventLoop.h
   ├─ InetAddress.h
   ├─ Poller.h
   ├─ Socket.h
   ├─ TimerWheel.h
   └─ base.h

```
## 参考
nginx
muduo
## TODO 
* Json 配置服务器
* 支持更多方法
* proactor(io_uring)模型和该模式的对比


