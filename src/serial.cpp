#include "serial.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdint.h>
#include <iostream>
#include <sys/ioctl.h>
#include <stdio.h>
#include <thread>

Serial::Serial() :
    _fd(-1),
    _connected(false)
{
}

bool Serial::connect(const std::string &device, unsigned int baud_rate)
{
    _device = device;
    _baudRate = baud_rate;
    // Open serial device for D6
    _fd = open(_device.c_str(), O_RDWR /* | O_NOCTTY | O_NDELAY */);
    if( _fd < 0 ) {
        std::cerr << "ERROR opening serial device " << device << " : " << strerror(errno) << std::endl;
        _connected = false;
        return false;
    }

    // fcntl(_fd, FNDELAY);

    // Configure serial port
    struct termios old_tios, new_tios;
    tcgetattr(_fd, &old_tios);
    memset(&new_tios, 0, sizeof(new_tios));
    new_tios.c_cflag = _baudRate | CRTSCTS | CS8 | CLOCAL | CREAD;
    new_tios.c_iflag = IGNPAR;
    new_tios.c_cc[VTIME] = 1;
    new_tios.c_cc[VMIN] = 200;
    //cfsetspeed(&new_tios, BAUD_RATE);
    tcflush(_fd, TCIFLUSH);
    tcsetattr(_fd, TCSANOW, &new_tios);

    _connected = true;
    return true;
}

Serial::~Serial()
{
    if(_connected) {
        std::cout << "Closing serial connection" << std::endl;
        close(_fd);
    }
}

int Serial::receive(unsigned char *buf, size_t buf_size)
{
    int read_bytes = read(_fd, buf, buf_size);
    if(read_bytes < 0) {
        perror("receive - read");
    }
    return read_bytes;
}

int Serial::receiveAll(unsigned char *buf, size_t buf_size)
{
    int read_bytes = 0;
    unsigned char *buf_ptr = buf;
    while(read_bytes < buf_size) {

        // Check bytes available to read in serial buffer
        int available_to_read;
        const int max_retries = 5;
        const int pause_in_ms_between_retries = 10;
        for(int i=0 ; i < max_retries ; i++) {
            ioctl(_fd, FIONREAD, &available_to_read);
            if(available_to_read == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(pause_in_ms_between_retries));
                continue;
            }
            else {
                break;
            }
        }
        if(available_to_read == 0) {
            //std::cout << "No more stuff to read" << std::endl;
            break;
        }

#define MAX_READ_BLOCK 1024ul
        size_t to_read = std::min(buf_size-read_bytes, MAX_READ_BLOCK);
        int r = read(_fd, buf_ptr, to_read);
        if(r < 0) {
            perror("receive - read");
            break;
        }
        else if (r == 0) {
            // This should not happen if we checked above that there are bytes available to read
            std::cout << "No more to read. Only " << read_bytes << " bytes read." << std::endl;
            break;
        }
        else {
            read_bytes += r;
            buf_ptr += r;
        }
    }
    return read_bytes;
}

int Serial::send(const std::string &cmd)
{
    int written = write(_fd, cmd.c_str(), cmd.size());
    if(written < 0) {
        perror("send - write");
    }
    fsync(_fd);
    return written;
}

std::string Serial::getFirstAvailableDevice(std::string format, int min, int max)
{
    for(int i=min ; i <= max ; i++) {
#define PATH_SIZE 128
        char path[PATH_SIZE];
        sprintf(path, format.c_str(), i);
        int fd = open(path, O_RDWR);
        if( fd < 0 ) {
            continue;
        }
        else {
            close(fd);
            return path;
        }
    }
    return "set serial device";
}
