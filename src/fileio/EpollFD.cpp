#include "EpollFD.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>

namespace fileio {

namespace detail {
std::map<unsigned int, std::string> event2Str = {
    {EPOLLIN, "EPOLLIN"},
    {EPOLLPRI, "EPOLLPRI"},
    {EPOLLOUT, "EPOLLOUT"},
    {EPOLLET, "EPOLLET"},
    {EPOLLHUP, "EPOLLHUP"},
    {EPOLLRDHUP, "EPOLLRDHUP"},
    {EPOLLERR, "EPOLLERR"},
    {EPOLLONESHOT, "EPOLLONESHOT"},
    {EPOLLWAKEUP, "EPOLLWAKEUP"},
    {EPOLLEXCLUSIVE, "EPOLLEXCLUSIVE"},
};
}

EpollFD::EpollFD() : FD(::epoll_create1(0)) {
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


EpollFD::EpollFD(EpollFD &&other) noexcept : FD(std::move(other)) {}

EpollFD& EpollFD::operator=(EpollFD &&other) noexcept {
    FD::operator =(std::move(other));
    return *this;
}

EpollFD::~EpollFD() {
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
void EpollFD::addFD(int _fd, Func cb, unsigned int _flags) {
    std::lock_guard lock {mutex};
    callbacks[_fd] = std::make_shared<Func>(std::move(cb));
    struct epoll_event event {};
    event.events = _flags;
    event.data.fd = _fd;
    if(::epoll_ctl(*this, EPOLL_CTL_ADD, _fd, &event) == -1) {
        callbacks.erase(_fd);
        throw std::runtime_error("cannot call epoll_ctl EPOLL_CTL_ADD " + std::string(std::strerror(errno)));
    }
}

void EpollFD::modFD(int _fd, int _flags) {
    struct epoll_event event {};
    event.events = _flags;
    event.data.fd = _fd;
    if (::epoll_ctl(*this, EPOLL_CTL_MOD, _fd, &event) == -1) {
        throw std::runtime_error("cannot call epoll_ctl EPOLL_CTL_MOD " + std::string(std::strerror(errno)));
    }
}

/* Caution when using rmFD() in callback context. Code right after rmFD() wont be valid anymore. */
void EpollFD::rmFD(int _fd) noexcept {
    std::lock_guard lock{mutex};
    if(callbacks.erase(_fd)) {
        if (::epoll_ctl(*this, EPOLL_CTL_DEL, _fd, nullptr) == -1) {
            /* this is not critical */
            std::cerr << "cannot call epoll_ctl EPOLL_CTL_DEL " << std::strerror(errno) << '\n';
        }
    }
}

std::vector<struct epoll_event> EpollFD::wait(int max_events, int timeout_ms) {
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

void EpollFD::dispatch(std::vector<struct epoll_event> const &events) {
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
        (*wrapper.cb)(wrapper.event.events);
    }
}

void EpollFD::work(int max_events, int timeout_ms) {
    dispatch(wait(max_events, timeout_ms));
}

/*
 * wakes up epoll_wait for every thread, so that the associated thread have the chance to end gracefully.
 */
void EpollFD::wakeup() {
    if(efd.valid()) {
        efd.put(1);
    }
}

std::string EpollFD::getEventsStr(int events) {
    std::stringstream ss;
    ss << "Events: " << std::hex << std::showbase << events << '\n';
    if(events != 0) {
        for(auto const& [event, name] : detail::event2Str) {
            if(events & event)
                ss << "  " << name << "\n";
        }
    }
    return ss.str();
}

}
