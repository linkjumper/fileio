#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>


struct ThreadPool {
    using Func = std::function<void(int)>;

    ThreadPool(unsigned int num_threads);
    ~ThreadPool();

    void enqueue(std::shared_ptr<Func> f, int arg);
    void stop();

private:
    /* in a real world application use run with load()/ store() std::memory_order_relaxed */
    std::atomic<bool> run{true};
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::queue<std::pair<std::shared_ptr<Func>, int>> tasks;
    std::condition_variable cv;
    bool new_data{false};
};
