
```
                                                   
                                                   
    ,---,                ,--,                 ,-.  
  .'  .' `\            ,--.'|             ,--/ /|  
,---.'     \           |  | :           ,--. :/ |  
|   |  .`\  |          :  : '           :  : ' /   
:   : |  '  | ,--.--.  |  ' |     ,---. |  '  /    
|   ' '  ;  :/       \ '  | |    /     \'  |  :    
'   | ;  .  .--.  .-. ||  | :   /    /  |  |   \   
|   | :  |  '\__\/: . .'  : |__.    ' / '  : |. \  
'   : | /  ; ," .--.; ||  | '.''   ;   /|  | ' \ \ 
|   | '` ,/ /  /  ,.  |;  :    '   |  / '  : |--'  
;   :  .'  ;  :   .'   |  ,   /|   :    ;  |,'     
|   ,.'    |  ,     .-./---`-'  \   \  /'--'       
'---'       `--`---'             `----'          
```
## 搭建
`chmod 777 -c build.sh`
`./build.sh`
## 用法
`./Dalek start [-p port] [-n worker_num]`

## 测试
测试机器处理器：`i5-9300` ，系统 `Ubuntu 20.0.4`
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
`Master/Worker`模型，端口复用，使用时间轮管理长连接 ，每个工作进程创建一个`EventLoop`和`HttpServer`，`HttpServer`负责接受连接，注册连接进入`EventLoop`和`TimerWheel`。

## Dalek细节
`master`进程主要负责杀死`worker`进程， 当`worker`进程挂了，就会唤醒`master`进程，再`fork()`一个`worker`
`httppar.h`实现了一个高性能的http解析器，可以在客户端40ms发送1byte的情况下正确的解析http请求包 ，`reactor`目录下则对事件进行了抽象，任何IO事件都封装成`Channel`，包括定时事件`TimerWheel`，服务器事件`HttpServer`，连接事件`HttpConnection`



| language | files | code | comment | blank | total | comment rate |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: |
| C++ | 18 | 2,374 | 262 | 703 | 3,339 | 9.94% |
| Markdown | 1 | 82 | 0 | 17 | 99 | 0.00% |
| Shell Script | 1 | 3 | 1 | 2 | 6 | 25.00% |

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



