#include "Socket.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "InetAddress.h"
#include "util.h"


Socket::Socket() : fd(-1) {
  fd = socket(AF_INET, SOCK_STREAM, 0);
  errif(fd < 0, "socket create error");
}

Socket::Socket(int _fd) : fd(_fd) { errif(fd < 0, "socket create error"); }

Socket::~Socket() {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}

void Socket::bind(InetAddress *_addr) {
  struct sockaddr_in addr = _addr->getAddr();
  socklen_t addr_len = _addr->getAddr_len();
  errif(::bind(fd, reinterpret_cast<sockaddr *>(&addr), addr_len) < 0, "socket bind error");
}

void Socket::listen() { errif(::listen(fd, SOMAXCONN) < 0, "socket listen error"); }

void Socket::setnonblocking() {
  int flags = fcntl(fd, F_GETFL, 0);
  errif(flags < 0, "fcntl get flags error");
  flags |= O_NONBLOCK;
  errif(fcntl(fd, F_SETFL, flags) < 0, "fcntl set non-blocking error");
}

int Socket::accept(InetAddress *_addr) {
  // for server socket, we need to accept the connection
  int clnt_sockfd = -1;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  bzero(&addr, addr_len);
  if (fcntl(fd, F_GETFL) & O_NONBLOCK) {
    while (true) {
      clnt_sockfd = ::accept(fd, reinterpret_cast<sockaddr *>(&addr), &addr_len);
      if (clnt_sockfd < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
        continue;  // No connections available, try again
      } else if (clnt_sockfd < 0) {
        errif(clnt_sockfd < 0, "socket accept error");
      } else {
        break;  // Successfully accepted a connection
      }
    }
  } else {
    clnt_sockfd = ::accept(fd, reinterpret_cast<sockaddr *>(&addr), &addr_len);
    errif(clnt_sockfd < 0, "socket accept error");
  }
  _addr->setInetAddr(addr, addr_len);
  return clnt_sockfd;  // Return the client socket file descriptor
}

void Socket::connect(InetAddress *_addr) {
  // for client socket, we need to connect to the server
  struct sockaddr_in addr = _addr->getAddr();
  if (fcntl(fd, F_GETFL) & O_NONBLOCK) {
    while (true) {
      int ret = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), _addr->getAddr_len());
      if (ret == 0) {
        return;  // Successfully connected
      } else if (errno == EINPROGRESS) {
        continue;  // Non-blocking connect in progress
                   /*
                           The socket is nonblocking and the connection cannot be
                       completed immediately.  (UNIX domain sockets failed with
                       EAGAIN instead.)  It is possible to select(2) or poll(2)
                       for completion by selecting the socket for writing.  After
                       select(2) indicates writability, use getsockopt(2) to read
                       the SO_ERROR option at level SOL_SOCKET to determine
                       whether connect() completed successfully (SO_ERROR is
                       zero) or unsuccessfully (SO_ERROR is one of the usual
                       error codes listed here, explaining the reason for the
                       failure).
                   */
      } else if (ret < 0 && errno != EINPROGRESS) {
        errif(true, "socket connect error");
      }
    }
  } else {
    errif(::connect(fd, reinterpret_cast<sockaddr *>(&addr),
        _addr->getAddr_len()) < 0 && errno != EINPROGRESS, "socket connect error");
    return;  // Non-blocking connect, just return
  }
}

int Socket::getFd() const { return fd; }