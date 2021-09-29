#pragma once

#include <cstdint>
#include "FD.h"

namespace fileio {

/* SignalFD is not easy to use for termination purposes with epoll. It would have
 * to be ensured that worker threads are always available so that further signals
 * can be handled. Worst case: a single blocking worker thread
 *
 * Please use plain c signal handler instead!
 */

struct SignalFD : FD {
    SignalFD(uint32_t _signum, int flags = 0);
    ~SignalFD() = default;
    SignalFD(SignalFD& other) = delete;
    SignalFD& operator=(SignalFD &other) = delete;
    SignalFD(SignalFD &&other) noexcept = default;
    SignalFD& operator=(SignalFD &&other) noexcept = default;

    void ack();

private:
    uint32_t signum;
};

}
