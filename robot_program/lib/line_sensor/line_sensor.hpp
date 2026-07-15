#ifndef _LINE_SENSOR_HPP_
#define _LINE_SENSOR_HPP_

#pragma once

#include <iostream>
#include <Arduino.h>
#include <cmath>
#include "bot_data.h"

#define TOTAL_BYTES (sizeof(BotData) + 2 * sizeof(float) + sizeof(bool)) // 2 floats for angle and distance, 1 int for communication success
class LineSensor {
    public:
    float distance, angle;
    BotData other_data;
    bool read_success;
    bool other_data_received;
    bool read_serial(uint8_t* result);
    void update(); // call update every loop to read serial
    void angle_correction(float heading);
    float get_distance();
    float get_angle();
    void send_bot_data(BotData self_data);
};

#endif