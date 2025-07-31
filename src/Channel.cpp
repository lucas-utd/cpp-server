#include "Channel.h"
#include "EventLoop.h"

#include <unistd.h>


Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop), fd(_fd), events(0), ready(0), inEpoll(false), useThreadPool(false) {
}

Channel::~Channel() {
    if (fd >= 0) {
        close(fd);
    }
    fd = -1;
}

void Channel::handleEvent() {
    if (ready & (EPOLLIN | EPOLLPRI)) {
        if (useThreadPool) {
            loop->addThread(readCallback);
        }
        else if (readCallback) {
            readCallback();
        }
    }
    if (ready & (EPOLLOUT)) {
        if (useThreadPool) {
            loop->addThread(writeCallback);
        }
        else if (writeCallback) {
            writeCallback();
        }
    }
}

void Channel::enableRead() {
    events |= EPOLLIN | EPOLLPRI;
    loop->updateChannel(this);
}

void Channel::useET() {
    events |= EPOLLET; // Enable edge-triggered mode
    loop->updateChannel(this);
}

int Channel::getFd() {
    return fd;
}

uint32_t Channel::getEvents() {
    return events;
}

uint32_t Channel::getReady() {
    return ready;
}

bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setInEpoll(bool _in) {
    inEpoll = _in;
}

void Channel::setReady(uint32_t _ev) {
    ready = _ev;
}

void Channel::setReadCallback(std::function<void()> cb) {
    readCallback = std::move(cb);
}

void Channel::setWriteCallback(std::function<void()> cb) {
    writeCallback = std::move(cb);
}

void Channel::setUseThreadPool(bool _use) {
    useThreadPool = _use;
}
