#pragma once

#include <string>
#include <termios.h>

#define DEFAULT_SERIAL_DEVICE "/dev/ttyUSB1"
#define DEFAULT_BAUD_RATE B57600


class Serial {

public:

    Serial();
    ~Serial();

    bool connect(const std::string &device=DEFAULT_SERIAL_DEVICE, unsigned int baud_rate=DEFAULT_BAUD_RATE);

    inline bool connected()
    { return _connected; }

    int send(const std::string &cmd);
    int receive(unsigned char *buf, size_t buf_size);
    int receiveAll(unsigned char *buf, size_t buf_size);

    static std::string getFirstAvailableDevice(std::string format="/dev/ttyUSB%d", int min=1, int max=10);

private:

    std::string _device;
    unsigned int _baudRate;
    int _fd;
    bool _connected;
};
