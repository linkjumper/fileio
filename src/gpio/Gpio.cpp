#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

#include "Gpio.h"
#include "utils/FileWriter.h"


namespace fileio {

namespace {
std::string const gpio_base_path = "/sys/class/gpio/";
}

Gpio::Gpio(int pinNumber, Direction direction, Sensitivity sensitivity) : 
        m_gpioNumber(pinNumber), m_gpioStr(std::to_string(m_gpioNumber)), m_direction(direction), m_sensitivity(sensitivity) {
    write_file(gpio_base_path + "export", m_gpioStr);
    setSensitivity(sensitivity);
    setDirection(direction);
    openGpioFd();
    update();
}

Gpio::~Gpio() {
    FD::close();
    write_file(gpio_base_path + "unexport", m_gpioStr);
}

void Gpio::openGpioFd() {
    if (m_direction == Direction::IN) {
        FD::operator=(::open((gpio_base_path + "gpio" + m_gpioStr + "/value").c_str(), O_RDONLY | O_NONBLOCK));
    } else {
        FD::operator=(::open((gpio_base_path + "gpio" + m_gpioStr + "/value").c_str(), O_RDWR | O_NONBLOCK));
    }
}

void Gpio::setDirection(Direction direction) {
    if (direction == Direction::IN) {
        write_file(gpio_base_path + "gpio" + m_gpioStr + "/direction", "in");
    } else {
        write_file(gpio_base_path + "gpio" + m_gpioStr + "/direction", "out");
    }
    m_direction = direction;
}

void Gpio::setSensitivity(Sensitivity sensitivity) {
    std::string pat = gpio_base_path + "gpio" + m_gpioStr + "/edge";
    switch (sensitivity) {
    case Sensitivity::NONE: write_file(pat, "none"); break;
    case Sensitivity::RISING: write_file(pat, "rising"); break;
    case Sensitivity::FALLING: write_file(pat, "falling"); break;
    case Sensitivity::BOTH: write_file(pat, "both"); break;
    }
    m_sensitivity = sensitivity;
}

void Gpio::update() {
    char state = '?';
    ::read(*this, &state, sizeof(state));
    ::lseek(*this, 0, SEEK_SET);
    switch (state) {
    case '0': m_value = false; break;
    case '1': m_value = true; break;
    default: throw std::runtime_error(std::string("the value of GPIO ") + std::to_string(m_gpioNumber) + " is invalid: " + state);
    }
}

Gpio& Gpio::operator=(bool rhs) {
    if (m_direction == Direction::OUT) {
        char state = rhs ? '1' : '0';
        ::write(*this, &state, sizeof(state));
        ::lseek(*this, 0, SEEK_SET);
        update();
    } else {
        throw std::runtime_error("cannot set the value of an input GPIO");
    }
    return *this;
}

}
