#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Acceptor.h"
#include "Connection.h"

#include <functional>
#include <unistd.h>

#define READ_BUFFER 1024

Server::Server(EventLoop* _loop) : loop(_loop), acceptor(nullptr) {
    acceptor = new Acceptor(loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server() {
    delete acceptor;
}


void Server::newConnection(Socket *sock) {
    if (sock->getFd() > 0){
        Connection *conn = new Connection(loop, sock);
        std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->SetDeleteConnectionCallback(cb);
        connections[sock->getFd()] = conn;
    }
}

void Server::deleteConnection(int sockfd) {
    if (sockfd > 0)
    {
        auto it = connections.find(sockfd);
        if (it != connections.end()) {
            delete it->second;
            connections.erase(it);
            printf("Connection with fd %d deleted\n", sockfd);
        }
    }
}