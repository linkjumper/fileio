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

void AsyncIO::workLoop(int nr_of_jobs) {
    while(run) {
        try {
            work(nr_of_jobs);
        } catch(std::exception const &e) {
            std::cerr << e.what();
        }
    }
}

void AsyncIO::workOneJobLoop() {
    workLoop(1);
}

bool AsyncIO::isTerminated() {
    return not run;
}

}
