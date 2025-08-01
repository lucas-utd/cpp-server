#include "Acceptor.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"

#include <stdio.h>

Acceptor::Acceptor(EventLoop *_loop) : loop_(_loop), channel_(nullptr) {
  sock_ = new Socket();
  InetAddress *addr = new InetAddress("127.0.0.1", 8888);
  sock_->Bind(addr);
  // sock_->setnonblocking();
  sock_->Listen();

  channel_ = new Channel(loop_, sock_->GetFd());
  std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
  channel_->SetReadCallback(cb);
  channel_->EnableRead();

  delete addr;  // Clean up the InetAddress object after binding
}

Acceptor::~Acceptor() {
  delete channel_;
  delete sock_;
}

void Acceptor::AcceptConnection() {
  InetAddress *clnt_addr = new InetAddress();
  Socket *clnt_sock = new Socket(sock_->Accept(clnt_addr));
  if (clnt_sock->GetFd() < 0) {
    delete clnt_addr;
    return;  // Accept failed
  }
  printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->GetFd(), inet_ntoa(clnt_addr->GetAddr().sin_addr),
         ntohs(clnt_addr->GetAddr().sin_port));
  clnt_sock->SetNonBlocking();

  if (new_connection_callback_) {
    new_connection_callback_(clnt_sock);
  }

  delete clnt_addr;
}

void Acceptor::SetNewConnectionCallback(const std::function<void(Socket *)>& _callback) {
  new_connection_callback_ = _callback;
}