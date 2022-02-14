// EventLoop
#ifndef PINK_EVENT_LOOP_H
#define PINK_EVENT_LOOP_H

#include <unistd.h>

#include <vector>

#include "Channel.h"
#include "Poller.h"

namespace pinkx {

class Channel;
class Poller;

class EventLoop : noncopyable {
 private:
  int pollTimeMs_;
  std::vector<Channel*> activeChannels_;
  bool quit_;
  Poller poller_;

 public:
  EventLoop();
  void loop();

  void update(Channel& c);
  void remove(Channel& c);
  int poll_time() const;
  void SetPollTime(int timeOut);
  void stop();
};

EventLoop::EventLoop() : quit_(false), poller_(this), pollTimeMs_(-1) {}

void EventLoop::loop() {
  while (!quit_) {
    // poll
    activeChannels_.clear();
    poller_.poll(pollTimeMs_, activeChannels_);
    for (auto& i : activeChannels_) {
      i->CallBack();
    }
  }
}

void EventLoop::update(Channel& c) { poller_.update(&c); }

void EventLoop::remove(Channel& c) { poller_.remove(&c); }

void EventLoop::SetPollTime(int ms) { pollTimeMs_ = ms; }

int EventLoop::poll_time() const { return pollTimeMs_; }

void EventLoop::stop() { quit_ = true; }

}  // namespace pinkx
#endif