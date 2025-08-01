#pragma once

class InetAddress;

class Socket {
 private:
  int fd;  // file descriptor for the socket

 public:
  Socket();
  Socket(int fd);
  ~Socket();

  void bind(InetAddress *addr);
  void listen();
  int accept(InetAddress *addr);

  void connect(InetAddress *addr);

  void setnonblocking();
  int getFd() const;
};