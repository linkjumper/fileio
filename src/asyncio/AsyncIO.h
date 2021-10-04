#pragma once

#include <thread>
#include <vector>
#include <atomic>

#include "../fileio/EpollFD.h"

namespace asyncio {

/* 
 * The AsyncIO load balancer has multiple worker threads (each calls
 * epoll_wait) waiting and compete for the next ready worker thread to process
 * the work in the form of a callback function on an file descriptor's epoll
 * event. Only one free worker thread processes the next pending event. The big
 * advantage of this implementation is that there is no bottleneck through a
 * main thread that has to dispatch the work to other worker threads.
 * 
 * Please note that one of the epoll flags EPOLLET or EPOLLONESHOT has to be
 * set with addFD() so that each event is fired exactly once.
 * 
 * How to use?
 *  - 7 worker threads
 *  - 1 main thread becomes also worker
 *  AsyncIO loop{7};
 *  loop.addFD(fd, [&](auto) {
 *      // handle work on fd event
 *  }, EPOLLIN | EPOLLET);
 *  loop.workOneJobLoop();
 * 
 */
struct AsyncIO : fileio::EpollFD {
    using EpollFD::operator=;
    using EpollFD::EpollFD;

    AsyncIO(int num_threads=std::thread::hardware_concurrency());
    ~AsyncIO();

    void stop();
    void workLoop(int nr_of_jobs);
    void workOneJobLoop();
    bool isTerminated();

protected:
    std::vector<std::thread> threads;
    std::atomic<bool> run{true};
};

}
