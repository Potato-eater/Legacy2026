#ifndef _IR_SENSOR_HPP_
#define _IR_SENSOR_HPP_

#pragma once

#include <iostream>
#include <Arduino.h>
#include <cmath>

class IRSensor {
    public:
    float magnitude, angle; // signal strength and angle relative to the robot.
    bool read_success; // if the data was successfully received.
    bool read_serial(float* result, int num_floats); // read from the camera in UART
    void update(); // call update every loop to read serial
    void angle_correction(float heading); // adjust for robot's current heading
    float get_magnitude();
    float get_angle();
};

#endif