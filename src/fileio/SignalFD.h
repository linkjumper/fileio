#pragma once

#include <sys/signalfd.h>
#include <stdexcept>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <iostream>

#include "FD.h"

namespace fileio {

/* SignalFD is not easy to use for termination purposes with epoll. It would have
 * to be ensured that worker threads are always available so that further signals
 * can be handled. Worst case: a single blocking worker thread
 *
 * Please use plain c signal handler instead!
 */

struct SignalFD : FD {
    SignalFD(uint32_t _signum, int flags = 0) : signum(_signum) {
        sigset_t mask{0};
        if(0 > ::sigaddset(&mask, signum)) {
            throw std::runtime_error("can not handle sigaddset " + std::strerror(errno));
        }
        if(0 > ::sigprocmask(SIG_BLOCK, &mask, NULL)) {
            throw std::runtime_error("can not handle sigprocmask" + std::strerror(errno));
        }
        FD::operator =(std::move(::signalfd(-1, &mask, flags | SFD_NONBLOCK)));
    }
    ~SignalFD() = default;
    SignalFD(SignalFD& other) = delete;
    SignalFD& operator=(SignalFD &other) = delete;
    SignalFD(SignalFD &&other) noexcept = default;
    SignalFD& operator=(SignalFD &&other) noexcept = default;

    void ack() {
        struct signalfd_siginfo si;
        int ret = ::read(*this, &si, sizeof(si));
        if(ret < 0) {
            std::cerr << std::strerror(errno) << '\n';
        }else if (ret != sizeof(si)) {
            std::cerr << "reading siginfo fails\n";
        }else if(signum != si.ssi_signo) {
            std::cerr << "received wrong signal: " << ::strsignal(si.ssi_signo) << '\n';
        }
    }

private:
    uint32_t signum;
};

}
