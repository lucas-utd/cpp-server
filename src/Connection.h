#pragma once
#include <functional>

class EventLoop;
class Socket;
class Channel;

class Connection {
    private:
        EventLoop *loop;
        Socket *sock;
        Channel *channel;
        std::function<void(Socket *)> deleteConnectionCallback;

    public:
        Connection(EventLoop *_loop, Socket *_socket);
        ~Connection();

        void echo(int sockfd);
        void SetDeleteConnectionCallback(std::function<void(Socket *)>);
};