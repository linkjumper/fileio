#include <string>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <chrono>
#include <signal.h>
#include <vector>
#include <cstring>

#include "fileio/EpollFD.h"
#include "asyncio/AsyncIO.h"
#include "fileio/TimerFD.h"

asyncio::AsyncIO loop {7};

void sigfunc(int signum) {
    static bool signalHandledBefore {false};
    std::cout << "\nReceived signal: " << ::strsignal(signum) << '\n';
    if (not signalHandledBefore) {
        signalHandledBefore = true;
        loop.stop();
    } else {
        std::cout << "terminating dirty\n";
        ::exit(EXIT_FAILURE);
    }
}

int main() {
    using namespace fileio;

    ::signal(SIGINT, sigfunc);
    ::signal(SIGTERM, sigfunc);

    std::cout << "start program ...\n";

    std::vector<TimerFD> vec;
    auto timeout = std::chrono::milliseconds {100};

    for (int i{0}; i<100; ++i) {
        vec.emplace_back(TimerFD{});

        /* add timer to asyncio loop */
        loop.addFD(vec[i], [&, i](auto) {
            std::cout << i << ". TimerFD becomes readable\n";
            loop.modFD(vec[i], EPOLLIN | EPOLLONESHOT);
            vec[i].reset(timeout);
        }, EPOLLIN | EPOLLONESHOT);

    }

    /* start all timer */
    for (auto &v : vec) {
        v.reset(timeout);
    }

    loop.workOneJobLoop();
    std::cout << "end program ... \n";
    return 0;
}

