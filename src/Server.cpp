#include "Server.h"

#include <unistd.h>

#include <thread>

#include "Acceptor.h"
#include "Channel.h"
#include "Connection.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "ThreadPool.h"
#include "util.h"

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : main_reactor_(_loop), acceptor_(nullptr), thread_pool_(nullptr) {
  acceptor_ = new Acceptor(main_reactor_);
  std::function<void(Socket *)> cb = std::bind(&Server::NewConnection, this, std::placeholders::_1);
  acceptor_->SetNewConnectionCallback(cb);

  int size = std::thread::hardware_concurrency();
  thread_pool_ = new ThreadPool(size);
  for (int i = 0; i < size; ++i) {
    sub_reactors_.push_back(new EventLoop());
  }

  for (int i = 0; i < size; ++i) {
    std::function<void()> sub_loop = std::bind(&EventLoop::Loop, sub_reactors_[i]);
    thread_pool_->Add(sub_loop);
  }
}

Server::~Server() {
  delete acceptor_;
  for (auto reactor : sub_reactors_) {
    delete reactor;
  }
  delete thread_pool_;
}

void Server::NewConnection(Socket *sock) {
    ErrorIf(sock->GetFd() < 0, "New connection error");
    int random = sock->GetFd() % sub_reactors_.size();
    Connection *conn = new Connection(sub_reactors_[random], sock);
    std::function<void(Socket *)> cb = std::bind(&Server::DeleteConnection, this, std::placeholders::_1);
    conn->SetDeleteConnectionCallback(cb);
    conn->SetOnConnectCallback(on_connect_callback_);
    connections_[sock->GetFd()] = conn;
}

void Server::DeleteConnection(Socket *sock) {
  if (sock->GetFd() > 0) {
    auto it = connections_.find(sock->GetFd());
    if (it != connections_.end()) {
      delete it->second;
      connections_.erase(it);
      printf("Connection with fd %d deleted\n", sock->GetFd());
    }
  }
}

void Server::OnConnect(std::function<void(Connection *)> _fn) { on_connect_callback_ = std::move(_fn); }