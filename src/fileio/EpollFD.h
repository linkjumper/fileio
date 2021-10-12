#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sys/epoll.h>

#include "EventFD.h"
#include "FD.h"

namespace fileio {

struct EpollFD : FD {
    using Func = std::function<void(int events)>;

    EpollFD();
    EpollFD(EpollFD &other) = delete;
    EpollFD& operator=(EpollFD &other) = delete;
    EpollFD(EpollFD &&other) noexcept;
    EpollFD& operator=(EpollFD &&other) noexcept;
    virtual ~EpollFD();

    void addFD(int _fd, Func cb, unsigned int _flags = EPOLLIN|EPOLLET);
    void modFD(int _fd, int _flags = EPOLLIN|EPOLLET);
    void rmFD(int _fd) noexcept;
    std::string getEventsStr(int events);

protected:
    void work(int max_events = 1, int timeout_ms = -1);
    void wakeup();
    std::vector<struct epoll_event> wait(int max_events = 32, int timeout_ms = -1);
    virtual void dispatch(std::vector<struct epoll_event> const &events);

    std::map<int, std::shared_ptr<Func>> callbacks {};
    std::shared_mutex mutex {};
private:

    EventFD efd{EFD_SEMAPHORE|EFD_NONBLOCK};
};

}
