#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sys/epoll.h>
#include <cerrno>
#include <cstring>

#include "EventFD.h"
#include "FD.h"

namespace fileio {

struct EpollFD : FD {
    using Func = std::function<void(int events)>;

    EpollFD() : FD(::epoll_create1(0)) {
        if (not valid()) {
            throw std::runtime_error("can not create epollfd");
        }

        if (not efd.valid()) {
            throw std::runtime_error("can not create eventfd");
        } else {
            /* Level triggered does not work for eventfd. Eventfds are always behave as EPOLLET, also EPOLLEXCLUSIVE is set implicit.
             * See https://stackoverflow.com/questions/62231433/how-to-use-an-eventfd-with-level-triggered-behaviour-on-epoll */
            addFD(efd, [&](auto) {
                efd.get();
            }, EPOLLIN);
        }
    }
    ;
    EpollFD(EpollFD &other) = delete;
    EpollFD& operator=(EpollFD &other) = delete;
    EpollFD(EpollFD &&other) noexcept :
            FD(std::move(other)) {
    }
    EpollFD& operator=(EpollFD &&other) noexcept {
        FD::operator =(std::move(other));
        return *this;
    }
    ~EpollFD() {
        std::lock_guard lock {mutex};
        for (auto const &it : callbacks) {
            if (::epoll_ctl(*this, EPOLL_CTL_DEL, it.first, nullptr) == -1) {
                std::cerr << "cannot call epoll_ctl EPOLL_CTL_DEL " << std::strerror(errno) << '\n';
            }
        }
        callbacks.clear();
    }

    /*
     * Note: When using multiple epoll_wait with the same epollfd,
     * one have to make sure only one epoll_event returns. So please use
     * either EPOLLET or EPOLLONESHOT.
     */
    void addFD(int _fd, Func cb, unsigned int _flags = EPOLLIN|EPOLLET) {
        std::lock_guard lock {mutex};
        callbacks[_fd] = std::move(cb);
        struct epoll_event event {};
        event.events = _flags;
        event.data.fd = _fd;
        if(::epoll_ctl(*this, EPOLL_CTL_ADD, _fd, &event) == -1) {
            callbacks.erase(_fd);
            throw std::runtime_error("cannot call epoll_ctl EPOLL_CTL_ADD " + std::string(std::strerror(errno)));
        }
    }

    void modFD(int _fd, int _flags = EPOLLIN|EPOLLET) {
        struct epoll_event event {};
        event.events = _flags;
        event.data.fd = _fd;
        if (::epoll_ctl(*this, EPOLL_CTL_MOD, _fd, &event) == -1) {
            throw std::runtime_error("cannot call epoll_ctl EPOLL_CTL_MOD " + std::string(std::strerror(errno)));
        }
    }

    /* Caution when using rmFD() in callback context. Code right after rmFD() wont be valid anymore. */
    void rmFD(int _fd) noexcept {
        std::lock_guard lock{mutex};
        if(callbacks.erase(_fd)) {
            if (::epoll_ctl(*this, EPOLL_CTL_DEL, _fd, nullptr) == -1) {
                /* this is not critical */
                std::cerr << "cannot call epoll_ctl EPOLL_CTL_DEL " << std::strerror(errno) << '\n';
            }
        }
    }

    std::vector<struct epoll_event> wait(int max_events = 32, int timeout_ms = -1) {
        std::vector <struct epoll_event> events(max_events, epoll_event{});
        int num = ::epoll_wait(*this, events.data(), events.size(), timeout_ms);

        if(num >= 0){
            events.resize(num);
            if(num == 0) {
                std::cout << " epoll_wait timeout occurs\n";
            }
        } else {
            /* when a signal interrupts this thread, EINTR returns. So ignore it. */
            if(errno != EINTR) {
                std::cerr << "epoll_wait: "<< std::strerror(errno) << '\n';
            }
            events.clear();
        }
        return events;
    }

    void dispatch(std::vector<struct epoll_event> const &events) {
        struct Wrapper {
            Func cb;
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
            wrapper.cb(wrapper.event.events);
        }
    }

    void work(int max_events = 1, int timeout_ms = -1) {
        dispatch(wait(max_events, timeout_ms));
    }

    /*
     * wakes up epoll_wait for every thread, so that the associated thread have the chance to end gracefully.
     */
    void wakeup() {
        if(efd.valid()) {
            efd.put(1);
        }
    }

    void printEvents(int events) {
        std::cout << "events: " << std::hex << std::showbase << events << '\n';
        if(events == 0) {
            return;
        }
        if(events & EPOLLIN) {
            std::cout << "  EPOLLIN\n";
        }
        if(events & EPOLLPRI) {
            std::cout << "  EPOLLPRI\n";
        }
        if(events & EPOLLOUT) {
            std::cout << "  EPOLLOUT\n";
        }
        if(events & EPOLLET) {
            std::cout << "  EPOLLET\n";
        }
        if(events & EPOLLHUP) {
            std::cout << "  EPOLLHUP\n";
        }
        if(events & EPOLLRDHUP) {
            std::cout << "  EPOLLRDHUP\n";
        }
        if(events & EPOLLERR) {
            std::cout << "  EPOLLERR\n";
        }
        if(events & EPOLLONESHOT) {
            std::cout << "  EPOLLONESHOT\n";
        }
        if(events & EPOLLWAKEUP) {
            std::cout << "  EPOLLWAKEUP\n";
        }
        if(events & EPOLLEXCLUSIVE) {
            std::cout << "  EPOLLEXCLUSIVE\n";
        }
    }

private:
    std::map<int, Func> callbacks;
    std::shared_mutex mutex;
    EventFD efd{EFD_SEMAPHORE|EFD_NONBLOCK};
};

}
