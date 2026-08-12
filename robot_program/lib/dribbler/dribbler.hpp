#ifndef _DRIBBLER_HPP_
#define _DRIBBLER_HPP_

#pragma once

#include <iostream>
#include <Arduino.h>
#include "pins.h"


// controls the dribbler motor
class DribblerMotor {
    public:
    int DIR_PIN, PWM_PIN;

    // constructor
    DribblerMotor(int dir_pin, int pwm_pin);

    // spin the dribbler
    void run(float speed=100);

    // spin the dribbler in reverse
    void run_reverse(float speed=100);

    // stop the dribbler
    void stop();
};

#endif