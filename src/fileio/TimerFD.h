#pragma once

#include <sys/timerfd.h>
#include <chrono>
#include "FD.h"

namespace fileio {

struct TimerFD : FD {
    TimerFD (int flags = TFD_NONBLOCK, int clockid = CLOCK_MONOTONIC);
    ~TimerFD() = default;
    TimerFD(TimerFD& other) = delete;
    TimerFD& operator=(TimerFD &other) = delete;
    TimerFD(TimerFD &&other) noexcept = default;
    TimerFD& operator=(TimerFD &&other) noexcept = default;

    /* call getElapsed() after timer expires, this is important for EPOLLET!
     * otherwise the fd does not fire again */
    uint64_t getElapsed() const;
    void cancel ();
    void reset(std::chrono::nanoseconds duration, bool oneshot = true);

private:
    constexpr struct timespec durationToTimespec(std::chrono::nanoseconds duration);
    constexpr std::chrono::nanoseconds timespecToDuration(struct timespec ts);
    std::chrono::nanoseconds getMonotonicTime();
};

}
