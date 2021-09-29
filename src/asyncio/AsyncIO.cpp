#include "AsyncIO.h"
#include <iostream>

namespace asyncio {

AsyncIO::AsyncIO(int num_threads) : EpollFD() {
    for (int i {0}; i < num_threads; ++i) {
        threads.emplace_back([&] {
            workOneJobLoop();
        });
    }
}

AsyncIO::~AsyncIO() {
    stop();
    for (auto& t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }
    threads.clear();
}

void AsyncIO::stop() {
    if(run) {
        run = false;
        for ([[maybe_unused]]auto const& t : threads) {
            wakeup();
        }
    }
}

void AsyncIO::workOneJobLoop() {
    while(run) {
        try {
            work(1);
        } catch(std::exception const &e) {
            std::cerr << e.what();
        }
    }
}

}
