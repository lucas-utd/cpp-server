#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void ErrorIf(bool condition, const char *message) {
  if (condition) {
    perror(message);
    exit(EXIT_FAILURE);
  }
}