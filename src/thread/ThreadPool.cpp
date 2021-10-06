#include "ThreadPool.h"

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <utility>

ThreadPool::ThreadPool(unsigned int num_threads) {
    for(unsigned int i{0}; i<num_threads; ++i) {
        threads.emplace_back(std::thread([&]{
            while(run) {
                std::shared_ptr<Func> f = nullptr;
                int arg;
                {
                    std::unique_lock<std::mutex> lock {mutex};
                    while(tasks.empty() and run) {
                        new_data = false;
                        cv.wait(lock, [&]{ return new_data;});
                    }

                    if(not run) {
                        return;
                    }

                    auto t = tasks.front();
                    tasks.pop();
                    f = t.first;
                    arg = t.second;
                }
                (*f)(arg);
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    stop();
    {
        std::lock_guard<std::mutex> lock {mutex};
        new_data = true;
    }
    cv.notify_all();
    for(auto &t : threads) {
        t.join();
    }
}

void ThreadPool::enqueue(std::shared_ptr<Func> f, int arg) {
    if(not run) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock {mutex};
        tasks.emplace(std::make_pair(f, arg));
        new_data = true;
    }
    cv.notify_one();
}

void ThreadPool::stop() {
    run = false;
}
