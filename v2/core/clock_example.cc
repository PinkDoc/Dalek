#include <iostream>
#include "event_module/dalek_eventloop.hpp"
#include "dalek_connection.hpp"


int main()
{
    core::connection::TimeOut = 64;

    core::EventLoop looper;
    core::Timer timer(looper);
    core::listener listener(looper, core::ip::address_v4::loopback(8000), timer);

    listener.SetAcceptCallBack([](core::listener& l, core::connection& c) {
        c.EnableRead();

        c.SetMsgCallBack([](core::connection& c ) {
            core::Buffer& wb = c.writeBuffer();
            core::Buffer& rb = c.readBuffer();
            wb.append(rb.peek(), rb.size());
            c.ReloadTimer();
            c.EnableWrite();
            c.DisableRead();
        });

        c.SetSendCallBack([](core::connection& c) {
            core::Buffer& wb = c.writeBuffer();
            core::Buffer& rb = c.readBuffer();
            rb.reset();
            c.EnableRead();
            c.DisableWrite();
        });

        c.SetTimeOutCallBack([](core::connection& c) {
            fprintf(stderr, "Connection is time out");
        });

    });

    looper.loop();
}