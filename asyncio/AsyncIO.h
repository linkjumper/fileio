#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

#include "../fileio/EpollFD.h"

namespace asyncio {

struct AsyncIO : fileio::EpollFD {
    using EpollFD::operator=;
    using EpollFD::EpollFD;

    AsyncIO(int num_threads=8) : EpollFD() {
        for (int i {0}; i < num_threads; ++i) {
            threads.emplace_back([&] {
                workOneJobLoop();
            });
        }
    }

    ~AsyncIO() {
        stop();
        for (auto& t : threads) {
            if(t.joinable()) {
                t.join();
            }
        }
        threads.clear();
    }

    void stop() {
        if(run) {
            run = false;
            for ([[maybe_unused]]auto const& t : threads) {
                wakeup();
            }
        }
    }

    void workOneJobLoop() {
        while(run) {
            try {
                work(1);
            } catch(std::exception const &e) {
                std::cerr << e.what();
            }
        }
    }

    std::vector<std::thread> threads;
    std::atomic<bool> run{true};
};

}
