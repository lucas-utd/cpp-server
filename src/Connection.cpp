#include "Connection.h"
#include "Buffer.h"
#include "Channel.h"
#include "Socket.h"
#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <functional>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop(_loop), sock(_sock), channel(nullptr), readBuffer(nullptr) {
  channel = new Channel(loop, sock->getFd());
  channel->enableRead();
  channel->useET();  // Use edge-triggered mode for better performance
  std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
  channel->setReadCallback(cb);
  readBuffer = new Buffer();
}

Connection::~Connection() {
  if (channel) {
    delete channel;
  }
  if (sock) {
    delete sock;
  }
  if (readBuffer) {
    delete readBuffer;
  }
}

void Connection::echo(int sockfd) {
  char buf[READ_BUFFER];
  while (true) {
    bzero(&buf, sizeof(buf));
    ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
    if (bytes_read > 0) {
      readBuffer->append(buf, bytes_read);
    } else if (bytes_read < 0 && errno == EINTR) {
      printf("continue reading");
      continue;
    } else if (bytes_read < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
      printf("finish reading once, errno: %d\n", errno);
      printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
      // errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) < 0, "socket write error");
      send(sockfd);
      readBuffer->clear();  // Clear the buffer after writing
      break;
    } else if (bytes_read == 0) {
      printf("EOF, client fd %d closed connection\n", sockfd);
      if (deleteConnectionCallback) {
        deleteConnectionCallback(sockfd);
      }
      break;  // Client closed connection
    } else {
      printf("Coonection reset by peer\n");
      if (deleteConnectionCallback) {
        deleteConnectionCallback(sockfd);
      }
      break;  // Handle other read errors
    }
  }
}

void Connection::SetDeleteConnectionCallback(std::function<void(int)> _cb) {
  deleteConnectionCallback = std::move(_cb);
}

void Connection::send(int sockfd) {
  char buf[readBuffer->size()];
  strcpy(buf, readBuffer->c_str());
  int data_size = readBuffer->size();
  int data_left = data_size;
  while (data_left > 0) {
    ssize_t bytes_write = write(sockfd, buf + (data_size - data_left), data_left);
    if (bytes_write < 0 && errno == EAGAIN) {
      break;
    }
    data_left -= bytes_write;
    if (bytes_write < 0) {
      perror("write error");
      break;
    }
  }
}