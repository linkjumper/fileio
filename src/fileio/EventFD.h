#pragma once

#include <sys/eventfd.h>
#include <cstring>

#include "FD.h"

namespace fileio {

struct EventFD: FD {
    EventFD(int flags = EFD_SEMAPHORE, int initval = 0) noexcept :
            FD(::eventfd(initval, flags)) {}
    ~EventFD() = default;
    EventFD(EventFD &other) = delete;
    EventFD& operator=(EventFD &other) = delete;
    EventFD(EventFD &&other) noexcept = default;
    EventFD& operator=(EventFD &&other) noexcept = default;

    uint64_t get() {
        uint64_t val {0};
        if (::eventfd_read(*this, &val) != 0) {
            std::cerr << "cannot read from eventfd: " << std::strerror(errno) << '\n';
        }
        return val;
    }

    void put(uint64_t val) {
        if (::eventfd_write(*this, val) != 0) {
            std::cerr << "cannot write to eventfd: " << std::strerror(errno) << '\n';
        }
    }
};

}
