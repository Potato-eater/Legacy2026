#ifndef _DRIBBLER_HPP_
#define _DRIBBLER_HPP_

#pragma once

#include <iostream>
#include <Arduino.h>
#include "pins.h"

class DribblerMotor {
    // motor spins a roller, roller spins the ball,
    // Boom. Dribbler.
    // these functions control the dribbler of the robot.
    // im pretty sure the function names are easy to understand.
    public:
    int DIR_PIN, PWM_PIN;
    DribblerMotor(int dir_pin, int pwm_pin);
    void run(float speed=100);
    void run_reverse(float speed=100);
    void stop();
};

#endif