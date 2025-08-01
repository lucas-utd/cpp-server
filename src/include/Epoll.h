#pragma once

#include <vector>

#include "Macros.h"

#ifdef OS_LINUX
#include <sys/epoll.h>
#endif

class Channel;

class Epoll {
 public:
  Epoll();
  ~Epoll();

  DISALLOW_COPY_AND_MOVE(Epoll);

  // void addFd(int fd, uint32_t op);
  void UpdateChannel(Channel *);
  void DeleteChannel(Channel *);

  std::vector<Channel *> Poll(int _timeout = -1);

 private:
  int epfd_{1};
  struct epoll_event *events_{nullptr};
};