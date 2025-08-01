#include "Socket.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "InetAddress.h"
#include "util.h"

Socket::Socket() : fd_(-1) {
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  ErrorIf(fd_ < 0, "socket create error");
}

Socket::Socket(int _fd) : fd_(_fd) { ErrorIf(fd_ < 0, "socket create error"); }

Socket::~Socket() {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
}

void Socket::Bind(InetAddress *_addr) {
  struct sockaddr_in tmp_addr = _addr->GetAddr();
  ErrorIf(::bind(fd_, reinterpret_cast<sockaddr *>(&tmp_addr), sizeof(tmp_addr)) < 0, "socket bind error");
}

void Socket::Listen() { ErrorIf(::listen(fd_, SOMAXCONN) < 0, "socket listen error"); }

void Socket::SetNonBlocking() {
  int flags = fcntl(fd_, F_GETFL, 0);
  ErrorIf(flags < 0, "fcntl get flags error");
  flags |= O_NONBLOCK;
  ErrorIf(fcntl(fd_, F_SETFL, flags) < 0, "fcntl set non-blocking error");
}

bool Socket::IsNonBlocking() const {
  int flags = fcntl(fd_, F_GETFL, 0);
  ErrorIf(flags < 0, "fcntl get flags error");
  return (flags & O_NONBLOCK) != 0;  // Check if the O_NONBLOCK flag is set
}

int Socket::Accept(InetAddress *_addr) {
  // for server socket, we need to accept the connection
  int clnt_sockfd = -1;
  struct sockaddr_in tmp_addr;
  socklen_t addr_len = sizeof(tmp_addr);
  bzero(&tmp_addr, addr_len);
  if (fcntl(fd_, F_GETFL) & O_NONBLOCK) {
    while (true) {
      clnt_sockfd = ::accept(fd_, reinterpret_cast<sockaddr *>(&tmp_addr), &addr_len);
      if (clnt_sockfd < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
        continue;  // No connections available, try again
      } else if (clnt_sockfd < 0) {
        ErrorIf(clnt_sockfd < 0, "socket accept error");
      } else {
        break;  // Successfully accepted a connection
      }
    }
  } else {
    clnt_sockfd = ::accept(fd_, reinterpret_cast<sockaddr *>(&tmp_addr), &addr_len);
    ErrorIf(clnt_sockfd < 0, "socket accept error");
  }
  _addr->SetAddr(tmp_addr);
  return clnt_sockfd;  // Return the client socket file descriptor
}

void Socket::Connect(InetAddress *_addr) {
  // for client socket, we need to connect to the server
  struct sockaddr_in tmp_addr = _addr->GetAddr();
  if (fcntl(fd_, F_GETFL) & O_NONBLOCK) {
    while (true) {
      int ret = ::connect(fd_, reinterpret_cast<sockaddr *>(&tmp_addr), sizeof(tmp_addr));
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
        ErrorIf(true, "socket connect error");
      }
    }
  } else {
    ErrorIf(::connect(fd_, reinterpret_cast<sockaddr *>(&tmp_addr), sizeof(tmp_addr)) < 0 && errno != EINPROGRESS,
          "socket connect error");
    return;  // Non-blocking connect, just return
  }
}

void Socket::Connect(const char *ip, uint16_t port) {
  // for client socket, we need to connect to the server
  InetAddress addr(ip, port);
  Connect(&addr);
}

int Socket::GetFd() const { return fd_; }