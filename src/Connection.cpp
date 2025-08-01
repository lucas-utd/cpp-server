#include "Connection.h"

#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <utility>
#include <functional>

#include "Buffer.h"
#include "Channel.h"
#include "Socket.h"
#include "util.h"
#include <sys/socket.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop_(_loop), sock_(_sock) {
    if (loop_ != nullptr) {
        channel_ = new Channel(loop_, sock_->GetFd());
        channel_->EnableRead();
        channel_->UseET();  // Use edge-triggered mode for better performance
    }
    read_buffer_ = new Buffer();
    send_buffer_ = new Buffer();
    state_ = State::Connected;  // Initial state
}

Connection::~Connection() {
    if (loop_ != nullptr) {
        if (channel_) {
            delete channel_;
        }
    }
    if (sock_) {
    delete sock_;
  }
  if (read_buffer_) {
    delete read_buffer_;
  }
    if (send_buffer_) {
        delete send_buffer_;
    }
}

void Connection::Read() {
    ASSERT(state_ == State::Connected, "Connection is not in a valid state to read");
    read_buffer_->Clear();  // Clear the read buffer before reading
    if (sock_->IsNonBlocking()) {
        ReadNonBlocking();
    } else {
        ReadBlocking();
    }
}

void Connection::Write() {
    ASSERT(state_ == State::Connected, "Connection is not in a valid state to write");
    if (sock_->IsNonBlocking()) {
        WriteNonBlocking();
    } else {
        WriteBlocking();
    }
    send_buffer_->Clear();  // Clear the send buffer after writing
}

void Connection::ReadNonBlocking() {
    int sockfd = sock_->GetFd();
    char buf[READ_BUFFER];
    while (true) {
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            read_buffer_->Append(buf, bytes_read);
        } else if (bytes_read < 0 && errno == EINTR) {
            printf("continue reading\n");
            continue;  // Interrupted, try reading again
        } else if (bytes_read < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            break;  // No more data to read, exit the loop
        } else if (bytes_read == 0) {
            printf("EOF, client fd %d closed connection\n", sockfd);
            state_ = State::Closed;  // Update state to Closed
            break;  // Client closed connection
        } else {
            printf("Other error on client fd %d\n", sockfd);
            state_ = State::Closed;  // Update state to Closed
            break;  // Handle other read errors
        }
    }
}

void Connection::WriteNonBlocking() {
    int sockfd = sock_->GetFd();
    char buf[send_buffer_->Size()];
    memcpy(buf, send_buffer_->ToStr(), send_buffer_->Size());
    int data_size = send_buffer_->Size();
    int data_left = data_size;
    while (data_left > 0) {
        ssize_t bytes_write = write(sockfd, buf + (data_size - data_left), data_left);
        if (bytes_write < 0 && errno == EINTR) {
            printf("continue writing\n");
            continue;  // Interrupted, try writing again
        }
        if (bytes_write < 0 && errno == EAGAIN) {
            break;  // No more space to write, exit the loop
        }
        if (bytes_write < 0) {
            printf("write error on client fd %d\n", sockfd);
            state_ = State::Closed;  // Update state to Closed
            break;  // Handle other write errors
        }
        data_left -= bytes_write;
    }
}

/**
 * Only used for client connections.
 */
void Connection::ReadBlocking() {
    int sockfd = sock_->GetFd();
    unsigned int rcv_size = 0;
    socklen_t len = sizeof(rcv_size);
    getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcv_size, &len);
    char buf[rcv_size];
    ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
    if (bytes_read > 0) {
        read_buffer_->Append(buf, bytes_read);
    } else if (bytes_read == 0) {
        printf("EOF, client fd %d closed connection\n", sockfd);
        state_ = State::Closed;  // Update state to Closed
    } else if (bytes_read < 0) {
        printf("Other error on client fd %d\n", sockfd);
        state_ = State::Closed;  // Update state to Closed
    }
}

/**
    * Only used for client connections.
 */
void Connection::WriteBlocking() {
    int scokfd = sock_->GetFd();
    ssize_t bytes_write = write(scokfd, send_buffer_->ToStr(), send_buffer_->Size());
    if (bytes_write < 0) {
        printf("write error on client fd %d\n", scokfd);
        state_ = State::Closed;  // Update state to Closed
    } else if (bytes_write == 0) {
        printf("No data written to client fd %d\n", scokfd);
    } else {
        printf("Wrote %zd bytes to client fd %d\n", bytes_write, scokfd);
    }
}

void Connection::Close() {
        if (delete_connection_callback_) {
            delete_connection_callback_(sock_);
        }
  }

Connection::State Connection::GetState() const {
  return state_;
}
void Connection::SetSendBuffer(const char *str) {
  send_buffer_->SetBuf(str);
}
Buffer *Connection::GetReadBuffer() const {
  return read_buffer_;
}
const char *Connection::ReadBuffer() const {
  return read_buffer_->ToStr();
}
Buffer *Connection::GetSendBuffer() const {
  return send_buffer_;
}
const char *Connection::SendBuffer() const {
  return send_buffer_->ToStr();
}

void Connection::SetDeleteConnectionCallback(std::function<void(Socket *)> const& _cb) {
  delete_connection_callback_ = std::move(_cb);
}

void Connection::SetOnConnectCallback(std::function<void(Connection *)> const& _cb) {
  on_connect_callback_ = std::move(_cb);
}

void Connection::GetlineSendBuffer() {
  send_buffer_->Getline();
}

Socket *Connection::GetSocket() const {
  return sock_;
}