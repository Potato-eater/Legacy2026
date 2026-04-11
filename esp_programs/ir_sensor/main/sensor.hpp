#ifndef SENSOR_HPP
#define SENSOR_HPP


#include <array>
extern "C" {
    #include <stdio.h>
    #include <stdint.h>
    #include <driver/gpio.h>
    #include <esp_system.h>
    #include <esp_task_wdt.h>
    #include <esp_timer.h>
    #include <math.h>
}
#include <algorithm>
#include <array>
#include <vector>
#include <iterator>
#include "calc.hpp"
class SensorRing {
    public:
    std::array<PolarVector, 16> sensor_vectors;
    std::array<uint8_t, 16> sorted_indexes;
    std::array<PolarVector, 16> sorted_vectors;
    SensorRing();
    void update();

    void get_sorted_vectors();

    PolarVector simpleRead();
    std::array<float, 4> advancedRead();
};


#endif