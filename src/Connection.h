#pragma once
#include <functional>
#include <string>

class EventLoop;
class Socket;
class Channel;
class Buffer;

class Connection {
    private:
        EventLoop *loop;
        Socket *sock;
        Channel *channel;
        std::function<void(Socket *)> deleteConnectionCallback;
        std::string *inBuffer;
        Buffer *readBuffer;

    public:
        Connection(EventLoop *_loop, Socket *_socket);
        ~Connection();

        void echo(int sockfd);
        void SetDeleteConnectionCallback(std::function<void(Socket *)>);
};