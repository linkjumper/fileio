#include "FD.h"
#include <unistd.h>

namespace fileio {

FD::FD(int _fd) : fd(_fd) {}

FD::FD(FD &&other) noexcept : fd(other.fd) {
    other.fd = -1;
}
FD& FD::operator=(FD &&other) noexcept {
    if (this != &other) {
        fd = other.fd;
        other.fd = -1;
    }
    return *this;
}

FD::~FD() {
    close();
}

bool FD::valid() const noexcept {
    return fd >= 0;
}

void FD::close() noexcept {
    if (not valid()) {
        ::close(*this);
        fd = -1;
    }
}

FD::operator int() const noexcept {
    return fd;
}

}
