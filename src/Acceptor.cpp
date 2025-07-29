#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Server.h"

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop) {
    sock = new Socket();
    addr = new InetAddress("127.0.0.1", 8080);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();

    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor() {
    delete acceptChannel;
    delete sock;
    delete addr;
}

void Acceptor::acceptConnection() {
    newConnectionCallback(sock);
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb) {
    newConnectionCallback = std::move(_cb);
}
