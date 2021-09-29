#pragma once

#include <thread>
#include <vector>
#include <atomic>

#include "../fileio/EpollFD.h"

namespace asyncio {

struct AsyncIO : fileio::EpollFD {
    using EpollFD::operator=;
    using EpollFD::EpollFD;

    AsyncIO(int num_threads=8);
    ~AsyncIO();

    void stop();
    void workOneJobLoop();

    std::vector<std::thread> threads;
    std::atomic<bool> run{true};
};

}
