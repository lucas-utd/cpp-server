#include "Epoll.h"
#include "Channel.h"
#include "util.h"
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 1000

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd < 0, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if (epfd >= 0) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}

// void Epoll::addFd(int fd, uint32_t op) {
//     struct epoll_event ev;
//     bzero(&ev, sizeof(ev));
//     ev.data.fd = fd;
//     ev.events = op;
//     errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0, "epoll add event error");
// }

std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds < 0, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        Channel *channel = static_cast<Channel*>(events[i].data.ptr);
        channel->setReady(events[i].events);
        activeEvents.push_back(channel);
    }
    return activeEvents;
}

void Epoll::updateChannel(Channel *channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0, "epoll add error");
        channel->setInEpoll();
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) < 0, "epoll modify error");
    }
}

void Epoll::deleteChannel(Channel* _channel) {
    int fd = _channel->getFd();
    errif(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) < 0, "epoll delete error");
    _channel->setInEpoll(false);
    _channel->setReady(0); // Reset ready state
}