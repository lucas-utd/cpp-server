#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd < 0, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    errif(bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0, "socket bind error");

    errif(listen(sockfd, SOMAXCONN) < 0, "socket listen error");

    int epfd = epoll_create1(0);
    errif(epfd < 0, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));

    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET; // Edge-triggered
    setnonblocking(sockfd);
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) < 0, "epoll ctl add socket fd error");

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds < 0, "epoll wait error");
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sockfd) {
                struct sockaddr_in clnt_addr;
                bzero(&clnt_addr, sizeof(clnt_addr));
                socklen_t clnt_addr_len = sizeof(clnt_addr);

                int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd < 0, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET; // Edge-triggered
                setnonblocking(clnt_sockfd);
                errif(epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev) < 0, "epoll ctl add client fd error");
            } else if (events[i].events & EPOLLIN) {
                char buf[READ_BUFFER];
                while (true) {
                    bzero(buf, sizeof(buf));
                    ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
                    if (bytes_read > 0) {
                        printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, bytes_read);
                    } else if (bytes_read < 0 && errno == EINTR) {
                        printf("read interrupted, retrying...\n");
                        continue; // Retry reading if interrupted by a signal
                    } else if (bytes_read < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
                        // No more data to read, break out of the loop
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    } else if (bytes_read == 0) {
                        // Client disconnected
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd);
                        break;
                    }
                }
            } else {
                printf("unknown event on fd %d\n", events[i].data.fd);
            }
        }
    }

    close(sockfd);
    return 0;
}