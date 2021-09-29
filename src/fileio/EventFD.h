#pragma once

#include <sys/eventfd.h>
#include <cstdint>

#include "FD.h"

namespace fileio {

struct EventFD: FD {
    EventFD(int flags = EFD_SEMAPHORE, int initval = 0) noexcept;
    ~EventFD() = default;
    EventFD(EventFD &other) = delete;
    EventFD& operator=(EventFD &other) = delete;
    EventFD(EventFD &&other) noexcept = default;
    EventFD& operator=(EventFD &&other) noexcept = default;

    uint64_t get();
    void put(uint64_t val);
};

}
