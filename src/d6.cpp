#include "d6.h"
#include <sstream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <chrono>
#include <thread>
#include <map>


std::map<long, std::string> units = {
    {1, "Hz"},
    {1e3, "kHz"},
    {1e6, "MHz"},
    {1e9, "GHz"}
};


D6::D6() :
    _serial()
{
}

bool D6::connect(const std::string &device, unsigned int baud_rate)
{
    return _serial.connect(device, baud_rate);
}

unsigned char D6::getVersion()
{
    _serial.send("\x8fv");
    unsigned char ver;
    _serial.receive(&ver, 1);
    return ver;
}

std::string D6::getVersionString()
{
    unsigned char ver = getVersion();
    std::stringstream str;
    str << ver / 100 << "." << ver % 100;
    return str.str();
}

const D6::Status &D6::queryStatus()
{
    _serial.send("\x8fs");
    unsigned char status[4];
    _serial.receive(status, 4);
    _status.variant = status[0];
    _status.attenuator = status[1];
    _status.converterAD = status[2] + (status[3] << 8);
    return _status;
}

const D6::Status &D6::getStatus()
{
    return _status;
}

void D6::setVFOFrequency(long freq)
{
#define SET_VFO_CMD_SIZE 13
    char cmd[SET_VFO_CMD_SIZE];
    sprintf(cmd, "\x8f\x66%09ld", freq/10);

    std::cout << "Sending command: '" << cmd << "'" << std::endl;
    _serial.send(cmd);
    std::cout << "Sent" << std::endl;
    // This command has no response
}

void D6::setInterval(long start_freq, long step_width, int number_of_samples, int delay, bool log)
{
    std::cout << "Interval: " << start_freq << " step width: " << step_width << " samples: " << number_of_samples << std::endl;
    // TO-DO: Check valid intervals
    _startFreq = start_freq/10;
    _stepWidth = step_width/10;
    _numberOfSamples = number_of_samples;
    _delay = delay;
    _logarithmic = log;

    _samples.resize(_numberOfSamples*2);
    _xValues.resize(_numberOfSamples);

    {
        // Compute multiple to use
        long center_freq = start_freq + (step_width * number_of_samples / 2);
        std::cout << "center freq.: " << center_freq << std::endl;
        auto it = units.lower_bound(center_freq);
        std::cout << "lower_bound " << it->first << std::endl;
        if(it == units.end()) {
            auto last = units.rbegin();
            _xMultiple = last->first;
        }
        else {
            if(it != units.begin())
                it--;
            _xMultiple = it->first;
        }
        std::cout << "center freq.: " << center_freq << " xmultiple: " << _xMultiple << std::endl;
    }

    float freq = start_freq;
    for(int i=0; i< _numberOfSamples; i++){
        _xValues[i] = freq / _xMultiple;
        freq += step_width;
    }
}

void D6::sweep()
{
    // After testing, with the firmware installed in the D6 as it came, only logarithmic modes 'a', 'c' and 'x' seem to work
    // Other modes return 0 measurements
#if 1
    char cmd_code = _logarithmic?'a':'b';    // a - log ; b - linear
    int audio = 0;

#define SWEEP_CMD_SIZE 29
    char cmd[SWEEP_CMD_SIZE];
    sprintf(cmd, "\x8f%c%09ld%08ld%04d%03d%02d", cmd_code, _startFreq, _stepWidth, _numberOfSamples, _delay, audio);
#elif 1
    char cmd_code = _logarithmic?'c':'d';    // c - log ; d - linear

#define SWEEP_CMD_SIZE 27
    char cmd[SWEEP_CMD_SIZE];
    sprintf(cmd, "\x8f%c%09ld%08ld%04d%03d", cmd_code, _startFreq, _stepWidth, _numberOfSamples, _delay);
#else
    // These codes are for other chips, apparently (x - AD8307 (log), w - AD8361 (linear RMS)
    char cmd_code = _logarithmic?'x':'w';

#define SWEEP_CMD_SIZE 23
    char cmd[SWEEP_CMD_SIZE];
    sprintf(cmd, "\x8f%c%09ld%08ld%04d", cmd_code, _startFreq, _stepWidth, _numberOfSamples);
#endif

    std::cout << "Sending command: '" << cmd << "'" << std::endl;
    _serial.send(cmd);
    std::cout << "Sent" << std::endl;
    memset((void*)_samples.data(), 0, 4*_numberOfSamples);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int received = _serial.receiveAll((unsigned char*)_samples.data(), 4*_numberOfSamples);
    std::cout << "Received " << received << " bytes" << std::endl;
}

const std::vector<float> D6::getXData()
{
    return _xValues;
}

const std::string &D6::getXUnits()
{
    return units[_xMultiple];
}

const std::vector<int16_t> D6::getYData()
{
    return _samples;
}
