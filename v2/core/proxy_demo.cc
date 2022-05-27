#include "dalek_connection.hpp"

using namespace core;



class proxy_server
{
private:
    EventLoop looper_;
    listener server_;

    std::vector<ip::address_v4> downservers_;

    ip::address_v4 randServerAddr();

    std::map<int, int> fuck_you;

    std::map<int, connection*> downConns_;

public:
    proxy_server(int port):
            server_(looper_, ip::address_v4::loopback(port))
    {
        server_.SetAcceptCallBack([&](listener& l, connection& c){
            tcp::tcpsocket s;
            ip::address_v4 down = randServerAddr();

            s.Connect(down);

            connection *downConn = new connection(looper_, std::move(s));

            fuck_you.insert({c.fd(), downConn->fd()});
            fuck_you.insert({downConn->fd(), c.fd()});
            downConns_.insert({downConn->fd(), downConn});

            c.EnableRead();
            c.EnableWrite();
            downConn->EnableRead();
            downConn->EnableWrite();

            c.SetMsgCallBack([this](connection& c) {
                auto fd = c.fd();
                auto down = fuck_you.at(fd);
                auto downConn = downConns_[down];
                downConn->writeBuffer().append(c.readBuffer().peek(), c.readBuffer().size());
            });

            downConn->SetMsgCallBack([this](connection& c) {
                auto fd = c.fd();
                auto client = fuck_you.at(fd);
                auto clientConn = downConns_[client];
                clientConn->writeBuffer().append(c.readBuffer().peek(), c.readBuffer().size());
            });

            c.SetCloseCallBack([this](connection& c) {
                downConns_.erase(c.fd());
            });

            downConn->SetMsgCallBack([this](connection& c) {
                downConns_.erase(c.fd());
            });

            c.SetErrorCallBack([this](connection& c){
                auto down = fuck_you.at(c.fd());
                downConns_.erase(down);
                downConns_.erase(c.fd());
            });

            downConn->SetErrorCallBack([this](connection& c) {
                auto client = fuck_you.at(c.fd());
                downConns_.erase(client);
                downConns_.erase(c.fd());
            });
        });
    }



};


int main()
{



}