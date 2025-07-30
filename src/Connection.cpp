#include "Connection.h"
#include "Socket.h"
#include "Channel.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <functional>

#define READ_BUFFER 1024

Connection::Connection(EventLoop* _loop, Socket* _sock) : loop(_loop), sock(_sock), channel(nullptr) {
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
}

Connection::~Connection() {
    if (channel) {
        delete channel;
    }
    if (sock) {
        delete sock;
    }
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (bytes_read < 0 && errno == EINTR) {
            printf("continue reading");
            continue;
        } else if (bytes_read < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else if (bytes_read == 0) {
            printf("EOF, client fd %d closed connection\n", sockfd);
            if (deleteConnectionCallback) {
                deleteConnectionCallback(sock);
            }
        }
    }
}

void Connection::SetDeleteConnectionCallback(std::function<void(Socket*)> cb) {
    deleteConnectionCallback = std::move(cb);
}