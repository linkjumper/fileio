#include "EventFD.h"

#include <sys/eventfd.h>
#include <cstring>
#include <iostream>


namespace fileio {

EventFD::EventFD(int flags, int initval) noexcept :
        FD(::eventfd(initval, flags)) {}

uint64_t EventFD::get() {
    uint64_t val {0};
    if (::eventfd_read(*this, &val) != 0) {
        std::cerr << "cannot read from eventfd: " << std::strerror(errno) << '\n';
    }
    return val;
}

void EventFD::put(uint64_t val) {
    if (::eventfd_write(*this, val) != 0) {
        std::cerr << "cannot write to eventfd: " << std::strerror(errno) << '\n';
    }
}

}
