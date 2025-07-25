#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

#include "util.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"


#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void handleReadEvent(int);


int main() {
    Socket *serv_sock = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    serv_sock->listen();
    Epoll *ep = new Epoll();
    serv_sock->setnonblocking();
    ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);
    while (true) {
        std::vector<epoll_event> events = ep->poll();
        int nfds = events.size();
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == serv_sock->getFd()) {
                // Handle the new client connection
                InetAddress *clnt_addr = new InetAddress();
                Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
                clnt_sock->setnonblocking();
                ep->addFd(clnt_sock->getFd(), EPOLLIN | EPOLLET);
                delete clnt_addr; // Clean up the InetAddress object
            } else if (events[i].events & EPOLLIN) {
                handleReadEvent(events[i].data.fd);
            } else {
                printf("unknown event on fd %d\n", events[i].data.fd);
            }
        }
    }

    delete serv_sock;
    delete serv_addr;
    return 0;
}

void handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (bytes_read < 0 && errno == EINTR) {
            printf("read interrupted, retrying...\n");
            continue; // Retry reading if interrupted by a signal
        } else if (bytes_read < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            printf("finish reading once, errno: %d\n", errno);
            break; // No more data to read, exit the loop
        } else if (bytes_read == 0) {
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); // Close the socket if client disconnected
            break;
        }
    }
}