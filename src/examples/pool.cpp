#include <string>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <chrono>
#include <signal.h>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <cstring>

#include "fileio/TimerFD.h"

#ifdef ASYNCIO_LOOP
#include "asyncio/AsyncIO.h"
asyncio::AsyncIO loop;
#else
#include "asyncio/LoadBalancer.h"
asyncio::LoadBalancer loop;
#endif

template <typename Func>
void timekeeping(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << "ms\n";
}

int main() {
    using namespace fileio;
    using namespace std::chrono_literals;

    std::atomic<int> c {0};
    std::vector<TimerFD> vec;
    auto timeout {50ms};

    for (int i{0}; i<512; ++i) {
        vec.emplace_back(TimerFD{});

        loop.addFD(vec[i], [&, i](auto) {

            /* do some heavy work */
            for(int j{0}; j<1000000; ++j) {
                asm("nop");
            }

            if(c++>10000 or loop.isTerminated()) {
                loop.stop();
                return;
            }

            loop.modFD(vec[i], EPOLLIN | EPOLLONESHOT);
            vec[i].reset(timeout);
        }, EPOLLIN | EPOLLONESHOT);
    }

    timekeeping([&]{
        /* start all timer */
        for (auto &v : vec) {
            v.reset(timeout);
        }

#ifdef ASYNCIO_LOOP
        loop.workOneJobLoop();
#else
        loop.workLoop(3);
#endif
    });

    return 0;
}

