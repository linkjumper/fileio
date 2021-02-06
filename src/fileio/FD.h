#pragma once

#include <unistd.h>

namespace fileio {

struct FD {
    FD(int _fd = -1) : fd(_fd) {}
    FD(const FD &other) = delete;
    FD& operator=(const FD &other) = delete;
    FD(FD &&other) noexcept {
        fd = other.fd;
        other.fd = -1;
    }
    FD& operator=(FD &&other) noexcept {
        if (this != &other) {
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }

    virtual ~FD() {
        close();
    }

    bool valid() const noexcept {
        return fd >= 0;
    }

    void close() noexcept {
        if (not valid()) {
            ::close(*this);
            fd = -1;
        }
    }

    operator int() const noexcept {
        return fd;
    }

private:
    int fd;
};

}
