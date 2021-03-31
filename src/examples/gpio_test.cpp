#include <string>
#include <iostream>
#include <unistd.h>
#include <signal.h>

#include "asyncio/AsyncIO.h"
#include "gpio/Gpio.h"


asyncio::AsyncIO loop {7};

void sigfunc(int signum) {
    static bool signalHandledBefore {false};
    std::cout << "\nReceived signal: " << ::strsignal(signum) << '\n';
    if (not signalHandledBefore) {
        signalHandledBefore = true;
        loop.stop();
    } else {
        std::cout << "terminating dirty\n";
        ::exit(EXIT_FAILURE);
    }
}

int main() {
    using namespace fileio;

    ::signal(SIGINT, sigfunc);
    ::signal(SIGTERM, sigfunc);

    fileio::Gpio gpio{3, fileio::Gpio::Direction::IN};

    std::cout << "starting program ...\n";

    loop.workOneJobLoop();
    std::cout << "end program ... \n";
    return 0;
}

