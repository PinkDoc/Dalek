#ifndef PINK_WHEEL_H
#define PINK_WHEEL_H

#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <sys/timerfd.h>
#include <unordered_map>

#include <string.h>
#include "Channel.h"
#include "EventLoop.h"


namespace pinkx {


class Slot {
private:
    std::vector<Channel*> connections_;
public:
    
    void tick()
    {
        for(auto& i : connections_) if (i) i->TimeOutCallBack();
        connections_.clear();
    }

    int insert(Channel& connection)
    {
        connections_.push_back(&connection);
        return connections_.size() - 1;
    }

    void del(int p) {
        connections_[p] = nullptr;
    }

};


 
class TimerWheel
{
private:
    int fd_;
    itimerspec howlong_;

    Channel channel_;
    EventLoop* looper_;
    std::vector<Slot> slots;           
    std::unordered_map<Channel*, std::pair<Slot*, int>> marker;

    size_t ptr;

    void create(Channel& connect, int timeOut) {
        int slotsLen = slots.size();
        int p = (ptr + timeOut) % slotsLen;
        int slotP = slots[p].insert(connect);
        marker.insert({&connect , {(slots.data() + p), slotP}});
    }

    void change(Channel& connect, int timeOut) {
        Slot* slot = marker[&connect].first;
        int p = marker[&connect].second;
        slot->del(p);
        int slotsLen = slots.size();
        p = (ptr + timeOut) % slotsLen;
        int slotP = slots[p].insert(connect); 
        marker[&connect].first = slots.data() + p;  
        marker[&connect].second = slotP;                                                                        
    }

public:


    TimerWheel(EventLoop& looper):
        fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
        ptr(0),
        slots(256),
        channel_(looper, fd_),
        looper_(&looper)
    {
        if (fd_ == -1) throw std::runtime_error {"TimerWheel::TimerWheel()"};
        bzero(&howlong_, sizeof howlong_);
        howlong_.it_value.tv_sec = 1;
        timerfd_settime(fd_, 0, &howlong_, nullptr);
        
        channel_.EnableRead();
        channel_.SetReadCallBack(
            std::bind(&TimerWheel::tick, this)
        );
        looper_->update(channel_);
    
    }



    TimerWheel(EventLoop& looper, size_t len):
        fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
        ptr(0),
        slots(len),
        channel_(looper, fd_),
        looper_(&looper)
    {
        if (fd_ == -1) throw std::runtime_error {"TimerWheel::TimerWheel()"};
        bzero(&howlong_, sizeof howlong_);
        howlong_.it_value.tv_sec = 1;
        timerfd_settime(fd_, 0, &howlong_, nullptr);

        channel_.EnableRead();
        channel_.SetReadCallBack(
            std::bind(&TimerWheel::tick, this)
        );

        looper_->update(channel_);
    }

    ~TimerWheel()
    {
        close(fd_);
    }
   
    // If active time
    void tick() {

        slots[ptr].tick();
        ptr = (ptr + 1) % slots.size(); 

        char buf[16];
        read(fd_, buf, 16);
        timerfd_settime(fd_, 0, &howlong_, nullptr);
    }

    // insert a new timer or change old timer
    void insert(Channel& connect, int timeOut)
    {
        if (marker.find(&connect) == marker.end())
        {
            create(connect, timeOut);
        } 
        else 
        {
            change(connect, timeOut);
        }
    }

    // delete timer
    void del(Channel& connect)
    {
        if (marker.find(&connect) != marker.end())
        {
            Slot* s = marker[&connect].first;
            int p = marker[&connect].second;
            s->del(p);  // set to nullptr
            marker.erase(&connect);
        }
    }
};



}

#endif