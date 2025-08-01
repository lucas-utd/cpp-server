#include "ThreadPool.h"

#include <stdexcept>

ThreadPool::ThreadPool(unsigned int _size) : stop_(false) {
  for (unsigned int i = 0; i != _size; ++i) {
    workers_.emplace_back(std::thread([this]() {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          condition_variable_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
          if (stop_ && tasks_.empty()) {
            return;  // Exit the thread if stopped and no tasks
          }
          task = std::move(tasks_.front());
          tasks_.pop();
        }
        task();
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;  // Set the stop flag to true
  }
  condition_variable_.notify_all();  // Notify all threads to wake up and exit
  for (std::thread &thread : workers_) {
    if (thread.joinable()) {
      thread.join();  // Wait for all threads to finish
    }
  }
}