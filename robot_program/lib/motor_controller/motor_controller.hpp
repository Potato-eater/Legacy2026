#ifndef _MOTOR_CONTROLLER_HPP_
#define _MOTOR_CONTROLLER_HPP_

#pragma once

#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "pins.h"
#include "vector.hpp"
#include "motor.hpp"

class MotorController {
    public:
    Motor TL;
    Motor TR;
    Motor BL;
    Motor BR;

    MotorController();
    void run_motors(float speed, float angle, float rotation); // run motors based on the desired speed, move angle and rotation
    void run_raw(float tl_raw, float tr_raw, float bl_raw, float br_raw); // run motors based on raw speeds for each motor
    void stop_motors(); // stop every drive motor

    private:
    std::array<float, 4> scale_speeds(std::array<float, 4> speeds, float scale_to); // scale speed ratios into the desired speed
    std::array<float, 4> get_motor_speeds(float movement_speed, float angle, float rotation); // calculate what speed each motor should run at.
};
#endif // MOTOR CONTROLLER