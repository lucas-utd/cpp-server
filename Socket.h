#pragma once

class InetAddress;

class Socket 
{
    private:
        int fd; // file descriptor for the socket

    public:
    Socket();
    Socket(int fd);
    ~Socket();

    void bind(InetAddress* addr);
    void listen();
    void setnonblocking();

    int accept(InetAddress* addr);

    int getFd() const;
};