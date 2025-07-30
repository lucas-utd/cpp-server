#include <iostream>
#include <unistd.h>
#include <string.h>
#include <functional>

#include "src/util.h"
#include "src/Buffer.h"
#include "src/Socket.h"
#include "src/InetAddress.h"
#include "src/ThreadPool.h"

void oneClient(int msgs, int wait) {
    Socket* sock = new Socket();
    InetAddress* addr = new InetAddress("127.0.0.1", 8888);
    sock->connect(addr);

    int sockfd = sock->getFd();

    Buffer* sendBuffer = new Buffer();
    Buffer* readBuffer = new Buffer();

    sleep(wait);
    int count = 0;
    while (count < msgs) {
        sendBuffer->setBuf("I am a client!");
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if (write_bytes < 0) {
            perror("socket already disconnected, can't write any more!\n");
            break;
        }

        int already_read = 0;
        char buf[1024];
        while (true) {
            bzero(buf, sizeof(buf));
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if (read_bytes > 0) {
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if (read_bytes == 0) {
                std::cout << "Server closed the connection." << std::endl;
                exit(EXIT_SUCCESS);
            }

            if (already_read >= sendBuffer->size()) {
                printf("count: %d, message from server read: %s\n", count, readBuffer->c_str());
                break;
            }
        }
    }
}"