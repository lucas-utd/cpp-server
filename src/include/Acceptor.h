#pragma once

#include <functional>

#include "Macros.h"

class EventLoop;
class Socket;
class Channel;

class Acceptor {
 public:
  explicit Acceptor(EventLoop *_loop);
  ~Acceptor();

  DISALLOW_COPY_AND_MOVE(Acceptor);

  void AcceptConnection();
  void SetNewConnectionCallback(std::function<void(Socket *)> const &_callback);

 private:
  EventLoop *loop_;
  Socket *sock_;
  Channel *channel_;
  std::function<void(Socket *)> new_connection_callback_;
};