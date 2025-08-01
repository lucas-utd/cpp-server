#include <string.h>
#include <unistd.h>
#include <iostream>

#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include "util.h"

#define BUFFER_SIZE 1024

int main() {
  Socket *sock = new Socket();
  errif(sock->getFd() < 0, "socket create error");
  InetAddress *addr = new InetAddress("127.0.0.1", 8888);
  sock->connect(addr);
  errif(sock->getFd() < 0, "socket connect error");

  Buffer *sendBuffer = new Buffer();
  Buffer *readBuffer = new Buffer();

  while (true) {
    sendBuffer->getline();
    if (sendBuffer->size() == 0) {
      printf("input is empty, try again...\n");
      continue;
    }

    ssize_t write_bytes = write(sock->getFd(), sendBuffer->c_str(), sendBuffer->size());
    if (write_bytes < 0) {
      printf("socket already disconnected, can't write any more!\n");
      break;
    }

    int already_read = 0;
    char buf[BUFFER_SIZE];
    while (true) {
      bzero(buf, sizeof(buf));
      ssize_t read_bytes = read(sock->getFd(), buf, sizeof(buf));
      if (read_bytes > 0) {
        readBuffer->append(buf, read_bytes);
        already_read += read_bytes;
      } else if (read_bytes == 0) {
        printf("server disconnected\n");
        exit(EXIT_SUCCESS);
      }

      if (already_read >= sendBuffer->size()) {
        printf("message from server: %s\n", readBuffer->c_str());
        break;  // Exit the loop after reading enough data
      }
    }
    readBuffer->clear();  // Clear the read buffer for the next iteration
    sendBuffer->clear();  // Clear the send buffer for the next input }
  }

  delete sendBuffer;
  delete readBuffer;
  delete addr;
  delete sock;
  return 0;
}