#pragma once

#include <functional>
#include <cstdint>

#include "Macros.h"

class Socket;
class EventLoop;

class Channel {
 public:
  Channel(EventLoop *_loop, int _fd);
  ~Channel();

  DISALLOW_COPY_AND_MOVE(Channel);

  void HandleEvent();
  void EnableRead();

  int GetFd() const;
  uint32_t GetEvents() const;
  uint32_t GetReady() const;
  bool GetInEpoll() const;
  void SetInEpoll(bool _in = true);
  void UseET();

  void SetReadyEvents(uint32_t _ready);
  void SetReadCallback(std::function<void()> const &_callback);
  void SetWriteCallback(std::function<void()> const &_callback);

 private:
  EventLoop *loop_;
  int fd_;
  uint32_t listen_events_;
  uint32_t ready_events_;
  bool in_epoll_;
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
};