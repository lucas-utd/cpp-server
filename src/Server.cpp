#include "Server.h"
#include "Acceptor.h"
#include "Channel.h"
#include "Connection.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "ThreadPool.h"

#include <unistd.h>
#include <functional>
#include <thread>

#define READ_BUFFER 1024

Server::Server(EventLoop *_loop) : mainReactor(_loop), acceptor(nullptr) {
  acceptor = new Acceptor(mainReactor);
  std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
  acceptor->setNewConnectionCallback(cb);

  int size = std::thread::hardware_concurrency();
  thpool = new ThreadPool(size);
  for (int i = 0; i < size; ++i) {
    subReactors.push_back(new EventLoop());
  }

  for (int i = 0; i < size; ++i) {
    std::function<void()> sub_loop = std::bind(&EventLoop::loop, subReactors[i]);
    thpool->add(sub_loop);
  }
}

Server::~Server() {
  delete acceptor;
  for (auto reactor : subReactors) {
    delete reactor;
  }
  delete thpool;
}

void Server::newConnection(Socket *sock) {
  if (sock->getFd() > 0) {
    int random = sock->getFd() % subReactors.size();
    Connection *conn = new Connection(subReactors[random], sock);
    std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->SetDeleteConnectionCallback(cb);
    connections[sock->getFd()] = conn;
  }
}

void Server::deleteConnection(int sockfd) {
  if (sockfd > 0) {
    auto it = connections.find(sockfd);
    if (it != connections.end()) {
      delete it->second;
      connections.erase(it);
      printf("Connection with fd %d deleted\n", sockfd);
    }
  }
}