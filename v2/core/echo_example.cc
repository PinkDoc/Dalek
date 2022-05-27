#include "dalek_logger.hpp"
#include "event_module/dalek_eventloop.hpp"
#include "dalek_connection.hpp"


int main()
{
    core::EventLoop looper;
    core::listener listener(looper, core::ip::address_v4::loopback(1989));
    Dalek_LoggerInit("echo.log");

    listener.SetAcceptCallBack([](core::listener& l, core::connection& c) {
        DLOG_INFO << "New connection coming!";
        c.EnableRead();

        c.SetMsgCallBack([](core::connection& c ) {
            core::Buffer& wb = c.writeBuffer();
            core::Buffer& rb = c.readBuffer();
            wb.append(rb.peek(), rb.size());
            c.EnableWrite();
            c.DisableRead();
        });

        c.SetSendCallBack([](core::connection& c) {
            DLOG_INFO << "Send Message to connection";
            core::Buffer& wb = c.writeBuffer();
            core::Buffer& rb = c.readBuffer();
            rb.reset();
            c.EnableRead();
            c.DisableWrite();
        });

    });

    looper.loop();
}