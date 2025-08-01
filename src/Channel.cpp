#include "Channel.h"

#include <unistd.h>
#include <sys/epoll.h>

#include "EventLoop.h"

Channel::Channel(EventLoop *_loop, int _fd) : loop_(_loop), fd_(_fd), listen_events_(0), ready_events_(0), in_epoll_(false) {}

Channel::~Channel() {
  if (fd_ >= 0) {
    close(fd_);
  }
  fd_ = -1;
}

void Channel::HandleEvent() {
  if (ready_events_ & (EPOLLIN | EPOLLPRI)) {
    if (read_callback_) {
      read_callback_();
    }
  }
  if (ready_events_ & (EPOLLOUT)) {
    if (write_callback_) {
      write_callback_();
    }
  }
}

void Channel::EnableRead() {
  listen_events_ |= EPOLLIN | EPOLLPRI;
  loop_->UpdateChannel(this);
}

void Channel::UseET() {
  listen_events_ |= EPOLLET;  // Enable edge-triggered mode
  loop_->UpdateChannel(this);
}

int Channel::GetFd() const { return fd_; }

uint32_t Channel::GetEvents() const { return listen_events_; }

uint32_t Channel::GetReady() const { return ready_events_; }

bool Channel::GetInEpoll() const { return in_epoll_; }

void Channel::SetInEpoll(bool _in) { in_epoll_ = _in; }

void Channel::SetReadyEvents(uint32_t _ev) { ready_events_ = _ev; }

void Channel::SetReadCallback(const std::function<void()> &_callback) { read_callback_ = _callback; }

void Channel::SetWriteCallback(const std::function<void()> &_callback) { write_callback_ = _callback; }