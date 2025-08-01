#include "InetAddress.h"
#include <string.h>

InetAddress::InetAddress(const char *ip, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = inet_addr(ip);
  addr_.sin_port = htons(port);
}

InetAddress::~InetAddress() {
  // Destructor implementation (if needed)
}

void InetAddress::SetAddr(sockaddr_in _addr) {
  addr_ = _addr;
}

sockaddr_in InetAddress::GetAddr() const { return addr_; }

const char *InetAddress::GetIp() const {
  return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::GetPort() const {
  return ntohs(addr_.sin_port);
}