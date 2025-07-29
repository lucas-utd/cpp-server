server:
	g++ src/util.cpp client.cpp -o client && \
	g++ server.cpp src/util.cpp src/Server.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp src/Channel.cpp src/EventLoop.cpp src/Acceptor.cpp -o server
clean:
	rm -f server client