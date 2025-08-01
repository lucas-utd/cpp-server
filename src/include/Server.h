#pragma once

#include <map>
#include <vector>
#include <functional>

#include "Macros.h"

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;

class Server {
 public:
  explicit Server(EventLoop *_loop);
  ~Server();

  DISALLOW_COPY_AND_MOVE(Server);

  void NewConnection(Socket *_sock);
  void DeleteConnection(Socket *_sock);
  void OnConnect(std::function<void(Connection *)> _fn);

 private:
  EventLoop *main_reactor_;
  Acceptor *acceptor_;
  std::map<int, Connection *> connections_;
  std::vector<EventLoop *> sub_reactors_;
  ThreadPool *thread_pool_;
  std::function<void(Connection *)> on_connect_callback_;
};