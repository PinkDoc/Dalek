<img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status">

![dw](dalek.png)

Dalek跑着我的小站：
[Dalek](http://www.pinkdoc.cn)
网站稳定运行中......
#### 搭建-Build
`chmod 777 -c build.sh`
`./build.sh`
#### 用法-Usage
目录下需要 `conf.json`文件
worker 是 工作进程数量，
port 则是 工作端口
conf.json
```
{
"worker" : [number]，
"port" : [port],
"root" : "[root direction]"
}
```
只需要：
`./Dalek`就可以使用

`./Dalek --usage` 查看如何使用

`./Dalek --version` 查看版本
#### 测压-Webbench
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

#### 实现-Implement
* 整体使用了是进程`master/worker`模型，每个进程执行一个事件循环， 主进程`master`负责生成`worker`进程，用户可以设置`worker`的数量，生成出相应的数量后，`master`就会`wait`阻塞状态。如果工作进程挂了，
就会唤醒`master`进程，`master`进程就会继续`fork`一个worker进程。同时，使用了`SO_REUSEPORT`来进行端口复用，内核做好了负载均衡 ：）, 避免了惊群效应。
* reactor模块封装了事件模型，`Channel`为基础事件，`Poller`则封装了`epoll`，可以用来执行进行回调，`Eventloop`则是`reactor`模型的核心，执行一次`epoll_wait`后返回活跃的事件，然后调用`Channel`相关的回调函数。同时 `TimerWheel`也被认为是一个事件，和`HttpConnection`, `HttpServer`一样。
* http模块封装了一个适用于非阻塞模型的 http解析器， 可以把一个http包解析，如果包不是完整的，它会保存当前状态，等收到完整的包为止。同时`HttpConnection`和`HttpServer`都是以`Channel`为核心，执行对应的callback操作。
* 日志库给每个进程一个缓冲区，如果缓冲区满了，就flush进日志文件（安全的）， 使用C风格， 给日志分了等级。
* 文件的传输用的是`sendfile`，提高了性能。
* 关于缓冲区`Buffer`的设计，使用vector来管理接受/发送的数据，使用一个`endIndex_`来标志buffer中数据的位置， 其中接受连接的数据比较有意思:
在栈上开辟 65535字节大小的空间，使用`readv`来分别读取，这样可以一次读更多的数据，而且一般情况下一个`tcp缓冲区`的大小为 8K（可以设置），所以这样读效率比较高。
* 内置json解析器 
#### 代码统计
* 语言

| language | files | code | comment | blank | total | comment rate |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: |
| C++ | 24 | 2,790 | 169 | 710 | 3,669 | 5.71% |
| Markdown | 1 | 99 | 0 | 14 | 113 | 0.00% |
| Shell Script | 1 | 3 | 1 | 2 | 6 | 25.00% |

| path | files | code | comment | blank | total | comment rate |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: |
| . | 26 | 2,892 | 170 | 726 | 3,788 | 5.55% |
| base | 8 | 270 | 32 | 92 | 394 | 10.60% |
| http | 6 | 1,299 | 85 | 273 | 1,657 | 6.14% |
| pson | 2 | 622 | 21 | 158 | 801 | 3.27% |
| reactor | 7 | 488 | 28 | 160 | 676 | 5.43% |

#### 代码树
```
Dalek
├─ CMakeLists.txt
├─ Dalek.cc
├─ Dalek.jpg
├─ README.md
├─ base
│  ├─ Logger.hpp
│  ├─ copyable.hpp
│  ├─ noncopyable.hpp
│  └─ swap.hpp
├─ build.sh
├─ http
│  ├─ base.hpp
│  ├─ buffer.hpp
│  ├─ httpd.hpp
│  ├─ httppar.hpp
│  ├─ httpres.hpp
│  └─ mime.hpp
├─ pson
│  ├─ Parser.hpp
│  └─ Value.hpp
├─ reactor
│  ├─ Channel.hpp
│  ├─ EventLoop.hpp
│  ├─ InetAddress.hpp
│  ├─ Poller.hpp
│  ├─ Socket.hpp
│  ├─ TimerWheel.hpp
│  └─ base.hpp
└─ tadis.jpg

```

#### 参考
nginx
muduo
#### TODO 
* 支持更多方法
* proactor(io_uring)模型和该模式的对比
* 一些细节



