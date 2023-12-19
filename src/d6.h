#pragma once

#include "serial.h"
#include <iostream>
#include <vector>

class D6 {
public:

    D6();

    bool connect(const std::string &device, unsigned int baud_rate);

    unsigned char getVersion();
    std::string getVersionString();

    void setVFOFrequency(long freq);

    void setInterval(long start_freq, long step_width, int number_of_samples, int delay, bool log);

    void sweep();

    inline int getNumberOfSamples() { return _numberOfSamples; }
    const std::vector<float> getXData();
    const std::string &getXUnits();
    const std::vector<int16_t> getYData();

    struct Status {
        uint8_t variant;
        uint8_t attenuator;
        int converterAD;

        void print()
        {
            std::cout << "Status -> variant: " << (int)variant << " att: " << (int)attenuator << " A/D converter: " << converterAD << std::endl;
        }
    };

    const Status &queryStatus();
    const Status &getStatus();

    inline bool connected() { return _serial.connected(); }

private:

    Serial _serial;

    Status _status;

    long _startFreq; ///< Start frequency in Hz
    long _stepWidth; ///< Step width in Hz
    int _numberOfSamples;   ///< Number of samples to capture
    int _delay; ///< Delay between measurements (microseconds)
    bool _logarithmic;

    std::vector<float> _xValues;
    float _xMultiple;
    std::vector<int16_t> _samples;

};
