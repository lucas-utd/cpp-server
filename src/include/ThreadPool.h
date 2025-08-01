#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Macros.h"

class ThreadPool {
 public:
  explicit ThreadPool(unsigned int _size = std::thread::hardware_concurrency());
  ~ThreadPool();

  DISALLOW_COPY_AND_MOVE(ThreadPool);

  template <typename F, typename... Args>
  auto Add(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

 private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable condition_variable_;
  bool stop_{false};
};

// Implementation Add template methods
// Because we are using templates, the implementation must be in the header file
// to allow the compiler to generate the necessary code for each instantiation.
template <typename F, typename... Args>
auto ThreadPool::Add(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task =
      std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_) {
      throw std::runtime_error("ThreadPool is stopped");
    }
    tasks_.emplace([task]() { (*task)(); });
  }
  condition_variable_.notify_one();
  return res;
}