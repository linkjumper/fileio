#include "SignalFD.h"

#include <sys/signalfd.h>
#include <stdexcept>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <iostream>

namespace fileio {

SignalFD::SignalFD(uint32_t _signum, int flags) : signum(_signum) {
    sigset_t mask{0};
    if(0 > ::sigaddset(&mask, signum)) {
        throw std::runtime_error("can not handle sigaddset " + std::string(std::strerror(errno)));
    }
    if(0 > ::sigprocmask(SIG_BLOCK, &mask, NULL)) {
        throw std::runtime_error("can not handle sigprocmask" + std::string(std::strerror(errno)));
    }
    FD::operator =(std::move(::signalfd(-1, &mask, flags | SFD_NONBLOCK)));
}

void SignalFD::ack() {
    struct signalfd_siginfo si;
    int ret = ::read(*this, &si, sizeof(si));
    if(ret < 0) {
        std::cerr << std::strerror(errno) << '\n';
    }else if (ret != sizeof(si)) {
        std::cerr << "reading siginfo failed\n";
    }else if(signum != si.ssi_signo) {
        std::cerr << "received wrong signal: " << ::strsignal(si.ssi_signo) << '\n';
    }
}

}
