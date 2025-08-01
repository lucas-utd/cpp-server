#include <string.h>
#include <unistd.h>
#include <functional>
#include <iostream>

#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include "ThreadPool.h"
#include "util.h"

void oneClient(int msgs, int wait) {
  Socket *sock = new Socket();
  InetAddress *addr = new InetAddress("127.0.0.1", 8888);
  sock->Connect(addr);

  int sockfd = sock->GetFd();

  Buffer *sendBuffer = new Buffer();
  Buffer *readBuffer = new Buffer();

  sleep(wait);
  int count = 0;
  while (count < msgs) {
    sendBuffer->SetBuf("I am a client!");
    ssize_t write_bytes = write(sockfd, sendBuffer->ToStr(), sendBuffer->Size());
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
        readBuffer->Append(buf, read_bytes);
        already_read += read_bytes;
      } else if (read_bytes == 0) {
        std::cout << "Server closed the connection." << std::endl;
        exit(EXIT_SUCCESS);
      }

      if (already_read >= sendBuffer->Size()) {
        printf("count: %d, message from server read: %s\n", count, readBuffer->ToStr());
        break;
      }
    }
    readBuffer->Clear();
  }
  delete sendBuffer;
  delete readBuffer;
  delete addr;
  delete sock;
}

int main(int argc, char *argv[]) {
  int threads = 100;
  int msgs = 100;
  int wait = 0;
  int o;

  const char *optstring = "t:m:w:";
  while ((o = getopt(argc, argv, optstring)) != -1) {
    switch (o) {
      case 't':
        threads = atoi(optarg);
        break;
      case 'm':
        msgs = atoi(optarg);
        break;
      case 'w':
        wait = atoi(optarg);
        break;
      case '?':
        printf("Usage: %s [-t threads] [-m messages] [-w wait]\n", argv[0]);
        printf("error optopt: %c\n", optopt);
        printf("error opterr: %d\n", opterr);
        break;
    }
  }

  ThreadPool *pool = new ThreadPool(threads);
  std::function<void()> func = std::bind(oneClient, msgs, wait);
  for (int i = 0; i < threads; ++i) {
    pool->Add(func);
  }
  delete pool;
  return 0;
}