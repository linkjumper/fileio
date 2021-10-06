#include "LoadBalancer.h"

#include <iostream>

namespace asyncio {

LoadBalancer::LoadBalancer(unsigned int nr_of_threads) :
    asyncio::AsyncIO(0), pool(std::make_unique<ThreadPool>(nr_of_threads)) {}

void LoadBalancer::dispatch(std::vector<struct epoll_event> const &events) {
    struct Wrapper {
        std::shared_ptr<Func> cb;
        struct epoll_event event;
    };

    std::vector<Wrapper> wrappers;

    {
        /* copy the callback handles as the callback map might be modified while accessing it */
        std::shared_lock lock{mutex};
        wrappers.reserve(events.size());
        for(auto const& event : events) {
            if(auto it = callbacks.find(event.data.fd); it !=callbacks.end()) {
                wrappers.emplace_back(Wrapper{callbacks[it->first], event});
            }
        }
    }

    for(auto const& wrapper : wrappers) {
        pool->enqueue(wrapper.cb, wrapper.event.events);
    }
}

}
