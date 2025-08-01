#pragma once

#include <functional>

#include "Macros.h"

class EventLoop;
class Socket;
class Channel;
class Buffer;

class Connection {
 public:

  enum class State {
  Invalid = 1,
  Handshaking,
  Connected,
  Closed,
  Failed
 };

  Connection(EventLoop *_loop, Socket *_socket);
  ~Connection();

  DISALLOW_COPY_AND_MOVE(Connection);

  void Read();
  void Write();

  void SetDeleteConnectionCallback(std::function<void(Socket *)> const &_callback);
  void SetOnConnectCallback(std::function<void(Connection *)> const &_callback);

  State GetState() const;
  void Close();
  void SetSendBuffer(const char *str);
  Buffer *GetReadBuffer() const;
  const char *ReadBuffer() const;
  Buffer *GetSendBuffer() const;
  const char *SendBuffer() const;
  void GetlineSendBuffer();
  Socket *GetSocket() const;

  void OnConnect(std::function<void()> fn);

 private:
  EventLoop *loop_;
  Socket *sock_;
  Channel *channel_{nullptr};
  State state_{State::Invalid};
  Buffer *read_buffer_{nullptr};
  Buffer *send_buffer_{nullptr};

  std::function<void(Socket *)> delete_connection_callback_;
  std::function<void(Connection *)> on_connect_callback_;

  void ReadNonBlocking();
  void WriteNonBlocking();
  void ReadBlocking();
  void WriteBlocking();
};