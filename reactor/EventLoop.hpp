// EventLoop
#ifndef PINK_EVENT_LOOP_HPP
#define PINK_EVENT_LOOP_HPP

#include <unistd.h>

#include <vector>

#include "Channel.hpp"
#include "Poller.hpp"

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

inline EventLoop::EventLoop() : quit_(false), poller_(this), pollTimeMs_(-1) {}

inline void EventLoop::loop() {
  while (!quit_) {
    // poll
    activeChannels_.clear();
    poller_.poll(pollTimeMs_, activeChannels_);
    for (auto& i : activeChannels_) {
      i->CallBack();
    }
  }
}

inline void EventLoop::update(Channel& c) { poller_.update(&c); }

inline void EventLoop::remove(Channel& c) { poller_.remove(&c); }

inline void EventLoop::SetPollTime(int ms) { pollTimeMs_ = ms; }

inline int EventLoop::poll_time() const { return pollTimeMs_; }

inline void EventLoop::stop() { quit_ = true; }

}  // namespace pinkx
#endif
