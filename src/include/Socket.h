#pragma once

#include <arpa/inet.h>

#include "Macros.h"

class InetAddress;

class Socket {
 public:
  Socket();
  explicit Socket(int fd);
  ~Socket();

  DISALLOW_COPY_AND_MOVE(Socket);

  void Bind(InetAddress *_addr);
  void Listen();
  int Accept(InetAddress *_addr);

  void Connect(InetAddress *_addr);
  void Connect(const char *ip, uint16_t port);

  void SetNonBlocking();
    bool IsNonBlocking() const;
  int GetFd() const;

 private:
  int fd_{1};  // file descriptor for the socket
};