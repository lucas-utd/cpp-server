#include "Socket.h"
#include "InetAddress.h"
#include "util.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>

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

void Socket::bind(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::bind(fd, (sockaddr*)&addr, addr_len) < 0, "socket bind error");
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

int Socket::accept(InetAddress* _addr) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    bzero(&addr, addr_len);
    int clnt_sockfd = ::accept(fd, (sockaddr*)&addr, &addr_len);
    errif(clnt_sockfd < 0, "socket accept error");
    _addr->setInetAddr(addr, addr_len);
    return clnt_sockfd;
}

void Socket::connect(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::connect(fd, (sockaddr*)&addr, addr_len) < 0, "socket connect error");
}

int Socket::getFd() const {
    return fd;
}