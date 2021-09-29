#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>


struct ThreadPool {
    using Func = std::function<void()>;

    ThreadPool(int num_threads);
    ~ThreadPool();

    void enqueue(std::shared_ptr<Func> f);
    void stop();

private:
    /* in a real world application use run with load()/ store() std::memory_order_relaxed */
    std::atomic<bool> run{true};
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::queue<std::shared_ptr<Func>> tasks;
    std::condition_variable cv;
    bool new_data{false};
};
