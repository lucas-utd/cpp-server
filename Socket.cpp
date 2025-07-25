#include "Socket.h"
#include "InetAddress.h"
#include "util.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd < 0, "socket create error");
}

Socket::Socket(int _fd): fd(_fd) {
    errif(fd < 0, "socket create error");
}

Socket::~Socket() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress* addr) {
    errif(::bind(fd, (sockaddr*)&addr->addr, addr->addr_len) < 0, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) < 0, "socket listen error");
}

void Socket::setnonblocking() {
    int flags = fcntl(fd, F_GETFL, 0);
    errif(flags < 0, "fcntl get flags error");
    flags |= O_NONBLOCK;
    errif(fcntl(fd, F_SETFL, flags) < 0, "fcntl set non-blocking error");
}

int Socket::accept(InetAddress* addr) {
    int clnt_sockfd = ::accept(fd, (sockaddr*)&addr->addr, &addr->addr_len);
    errif(clnt_sockfd < 0, "socket accept error");
    return clnt_sockfd;
}

int Socket::getFd() const {
    return fd;
}