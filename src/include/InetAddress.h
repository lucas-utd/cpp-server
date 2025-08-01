#pragma once

#include <arpa/inet.h>

#include "Macros.h"

class InetAddress {
 public:
  InetAddress() = default;
  InetAddress(const char *ip, uint16_t port);
  ~InetAddress();

  DISALLOW_COPY_AND_MOVE(InetAddress);

  void SetAddr(sockaddr_in _addr);
  sockaddr_in GetAddr() const;
  const char *GetIp() const;
  uint16_t GetPort() const;

 private:
  struct sockaddr_in addr_;
};