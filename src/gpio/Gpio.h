#pragma once

#include "fileio/FD.h"

namespace fileio {

class Gpio final : public FD {
public:
    enum class Direction { IN, OUT };

    enum class Sensitivity {
        NONE    = 0,
        RISING  = 1,
        FALLING = 2,
        BOTH    = RISING | FALLING,
    };

    Gpio(int pinNumber, Direction direction, Sensitivity sensitivity = Sensitivity::NONE);
    virtual ~Gpio();

    void        update();
    operator    bool() const { return m_value; }
    Gpio&       operator=(bool rhs);
    Direction   getDirection() const { return m_direction; }
    Sensitivity getSensitivity() const { return m_sensitivity; }
    int         getGpioNumber() const { return m_gpioNumber; }

private:
    void        setDirection(Direction direction);
    void        setSensitivity(Sensitivity sensitivity);
    void        openGpioFd();

    int               m_gpioNumber;
    const std::string m_gpioStr;
    Direction         m_direction;
    Sensitivity       m_sensitivity;
    bool              m_value;
};

}
