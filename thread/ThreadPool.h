#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>


struct ThreadPool {
    using Func = std::function<void()>;

    ThreadPool(int num_threads) {
        for(int i{0}; i<num_threads; ++i) {
            threads.emplace_back(std::thread([&]{
                while(run) {
                    std::shared_ptr<Func> f = nullptr;
                    {
                        std::unique_lock<std::mutex> lock {mutex};
                        while(tasks.empty() and run) {
                            new_data = false;
                            cv.wait(lock, [&]{ return new_data;});
                        }

                        if(not run) {
                            return;
                        }

                        f = tasks.front();
                        tasks.pop();
                    }
                    (*f)();
                }
            }));
        }
    }

    ~ThreadPool() {
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

    void enqueue(std::shared_ptr<Func> f) {
        if(not run) {
            return;
        }

        {
            std::lock_guard<std::mutex> lock {mutex};
            tasks.emplace(f);
            new_data = true;
        }
        cv.notify_one();
    }

    void stop() {
        run = false;
    }

private:
    /* in a real world application use run with load()/ store() std::memory_order_relaxed */
    std::atomic<bool> run{true};
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::queue<std::shared_ptr<Func>> tasks;
    std::condition_variable cv;
    bool new_data{false};
};
