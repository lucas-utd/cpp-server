#include "ThreadPool.h"

#include <stdexcept>

ThreadPool::ThreadPool(int size) : stop(false) {
    for (int i = 0; i < size; ++i) {
        threads.emplace_back(std::thread([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mtx);
                    cv.wait(lock, [this]() {
                        return stop || !tasks.empty();
                    });
                    if (stop && tasks.empty()) {
                        return; // Exit the thread if stopped and no tasks
                    }
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        stop = true; // Set the stop flag to true
    }
    cv.notify_all(); // Notify all threads to wake up and exit
    for (std::thread &thread : threads) {
        if (thread.joinable()) {
            thread.join(); // Wait for all threads to finish
        }
    }
}

void ThreadPool::add(std::function<void()> func) {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        if (stop) {
            throw std::runtime_error("ThreadPool is stopped, cannot add new tasks");
        }
        tasks.push(std::move(func)); // Add the task to the queue
    }
    cv.notify_one(); // Notify one thread to wake up and process the task
}