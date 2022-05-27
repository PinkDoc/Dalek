[中文](https://github.com/PinkDoc/Dalek/blob/master/README_cn.md)




This is a simple static http file server, 

it can maintain 100,000 levels of inactive connections(i5-9th),

the overall use of master/worker architecture,

I did this to learn network programming.

You can use it to submit school assignments, just give me a star(or fork)!

___
Q: How to use ?
A: You need to add a conf.json file to its directory
conf.json:
```
{
"worker":8,
"port":8080,
"root":"./"
}
```
then you put file in "root" directory, the port is 8080, and it will create 8 process to work.

___


Q: How does it work?

A: This is how eventloop work
```
while (!quit)
{
  epoll_wait(events);   // the process waits for the active events
  // After epoll wait
  for (auto& i : events)
  {
    if (i == listenfd) 
    {
      i.connection();       // It will add event to events
      continue;
    }
    
    // if i != listenfd
    if (i.event == READ) i.read();
    else if (i.event == WRITE) i.write();
    else if (i.event == ERROR) i.error();
    else if (i.event == TIMEOUT) i.timeout();
  }
}
```
And what is master/worker?
```
worker process creates an eventloop,
and the master process wait for the worker process die,
if worker die, master will fork a new worker.
```
