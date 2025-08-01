#include "Epoll.h"

#include <unistd.h>

#include <cstring>

#include "Channel.h"
#include "util.h"

#define MAX_EVENTS 1000

Epoll::Epoll() : epfd_(-1), events_(nullptr) {
  epfd_ = epoll_create1(0);
  ErrorIf(epfd_ < 0, "epoll create error");
  events_ = new epoll_event[MAX_EVENTS];
  bzero(events_, sizeof(*events_) * MAX_EVENTS);
}

Epoll::~Epoll() {
  if (epfd_ >= 0) {
    close(epfd_);
    epfd_ = -1;
  }
  delete[] events_;
}

// void Epoll::addFd(int fd, uint32_t op) {
//     struct epoll_event ev;
//     bzero(&ev, sizeof(ev));
//     ev.data.fd = fd;
//     ev.events = op;
//     ErrorIf(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0, "epoll add event error");
// }

std::vector<Channel *> Epoll::Poll(int timeout) {
  std::vector<Channel *> activeEvents;
  int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
  ErrorIf(nfds < 0, "epoll wait error");
  for (int i = 0; i < nfds; ++i) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    channel->SetReadyEvents(events_[i].events);
    activeEvents.push_back(channel);
  }
  return activeEvents;
}

void Epoll::UpdateChannel(Channel *channel) {
  int fd = channel->GetFd();
  struct epoll_event ev;
  bzero(&ev, sizeof(ev));
  ev.data.ptr = channel;
  ev.events = channel->GetEvents();
  if (!channel->GetInEpoll()) {
    ErrorIf(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0, "epoll add error");
    channel->SetInEpoll();
  } else {
    ErrorIf(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0, "epoll modify error");
  }
}

void Epoll::DeleteChannel(Channel *_channel) {
  int fd = _channel->GetFd();
  ErrorIf(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) < 0, "epoll delete error");
  _channel->SetInEpoll(false);
  _channel->SetReadyEvents(0);  // Reset ready state
}