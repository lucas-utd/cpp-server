#include <iostream>
#include <string>

#include "ThreadPool.h"

void print(int a, double b, const char *c, std::string d) {
  std::cout << "a: " << a << ", b: " << b << ", c: " << c << ", d: " << d << std::endl;
}

void test() { std::cout << "Testing ThreadPool" << std::endl; }

int main(int argc, char *argv[]) {
  ThreadPool *pool = new ThreadPool();
  std::function<void()> func = std::bind(print, 1, 3.14, "Hello", std::string("World"));
  pool->Add(func);
  func = test;
  pool->Add(func);
  delete pool;
  return 0;
}