#pragma once

#include <unistd.h>

namespace fileio {

struct FD {
    FD(int _fd = -1);
    FD(const FD &other) = delete;
    FD& operator=(const FD &other) = delete;
    FD(FD &&other) noexcept;
    FD& operator=(FD &&other) noexcept;
    virtual ~FD();

    bool valid() const noexcept;
    void close() noexcept;
    operator int() const noexcept;

private:
    int fd;
};

}
