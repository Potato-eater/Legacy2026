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
    float distance, angle; // the distance and direction to the line (if detected)
    BotData other_data; // data from the other robot
    bool read_success; // if the line sensor data was successfully received
    bool other_data_received; // if the other robot sent data
    bool read_serial(uint8_t* result); // read the UART
    void update(); // call update every loop to read serial
    void angle_correction(float heading); // account for robot heading
    float get_distance();
    float get_angle();
    void send_bot_data(BotData self_data); // send data over to the other robot
};
#endif