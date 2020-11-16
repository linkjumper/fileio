#pragma once

#include <sys/timerfd.h>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include "FD.h"

namespace fileio {

struct TimerFD : FD {
    TimerFD (int flags = TFD_NONBLOCK, int clockid = CLOCK_MONOTONIC): FD(::timerfd_create(clockid, flags)) {
        if (*this == -1) {
            throw std::runtime_error("can not create timerfd: " + std::string(std::strerror(errno)));
        }
    }
    ~TimerFD() = default;
    TimerFD(TimerFD& other) = delete;
    TimerFD& operator=(TimerFD &other) = delete;
    TimerFD(TimerFD &&other) noexcept = default;
    TimerFD& operator=(TimerFD &&other) noexcept = default;

    /* call getElapsed() after timer expires, this is important for EPOLLET!
     * otherwise the fd does not fire again */
    uint64_t getElapsed() const {
        uint64_t laps;
        ::read(*this,&laps, sizeof(laps));
        return laps;
    }

    void cancel () {
        reset(std::chrono::nanoseconds{0});
    }

    void reset(std::chrono::nanoseconds duration, bool oneshot = true) {
        struct itimerspec new_value{};

        if(duration != std::chrono::nanoseconds::zero()) {
            new_value.it_value = durationToTimespec(duration + getMonotonicTime());
            if(not oneshot) {
                new_value.it_interval = durationToTimespec(duration);
            }
        }

        if(::timerfd_settime(*this, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
            throw std::runtime_error("can not set timeout for timer: " + std::string(std::strerror(errno)));
        }
    }

private:
    constexpr struct timespec durationToTimespec(std::chrono::nanoseconds duration) {
        auto seconds   = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto nanoseconds   = duration - seconds;
        return timespec {seconds.count(), nanoseconds.count()};
    }

    constexpr std::chrono::nanoseconds timespecToDuration(struct timespec ts) {
        auto duration = std::chrono::seconds{ts.tv_sec} + std::chrono::nanoseconds{ts.tv_nsec};
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    }

    std::chrono::nanoseconds getMonotonicTime() {
        struct timespec now;
        if (::clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
            throw std::runtime_error("can not call clock_gettime: " + std::string(std::strerror(errno)));
        }
        return timespecToDuration(now);
    }
};

}
