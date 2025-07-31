src=$(wildcard src/*.cpp)

server:
	g++ -pthread $(src) server.cpp -o server

client:
	g++ -pthread $(src) client.cpp -o client

th:
	g++ -pthread ThreadPoolTest.cpp src/ThreadPool.cpp -o ThreadPoolTest

test:
	g++ -pthread $(src) test.cpp -o test

clean:
	rm -f server client ThreadPoolTest test