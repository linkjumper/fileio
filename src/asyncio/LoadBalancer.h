#pragma once

#include "AsyncIO.h"
#include "thread/ThreadPool.h"
#include <iostream>
#include <memory>

namespace asyncio {
/* 
 * The LoadBalancer is based on a thread pool. In contrast to the AsyncIO
 * load balancer, all jobs/events are dispatched in the main thread to the
 * worker threads. So only one epoll_wait is called from the main thread.
 * 
 * Please note: std::thread::hardware_concurrency() worker threads are used
 * as default!
 * 
 * How to use?
 *  - 7 worker threads
 *  - 1 main thread dispatch 3 jobs at once
 *  LoadBalancer loop{7};
 *  loop.addFD(fd,[&](auto) {
 *      // handle  work on fd event
 *  }, EPOLLIN | EPOLLET);
 *  loop.workLoop(3);
 * 
 */
struct LoadBalancer : asyncio::AsyncIO {
    LoadBalancer(unsigned int nr_of_threads = std::thread::hardware_concurrency());
    ~LoadBalancer() = default;
    void workOneJobLoop();

private:
    void dispatch(std::vector<struct epoll_event> const &events) override;

private:
    std::unique_ptr<ThreadPool> pool{nullptr};
};

}
