#ifndef _OTOS_HPP_
#define _OTOS_HPP_

#pragma once

#include <iostream>
#include <SparkFun_Qwiic_OTOS_Arduino_Library.h>
#include "vector.hpp"
// using the sparkfun optical tracking odometry sensor (mouse sensor),
// we can accurately find the location of the robot without needing to
// rely on distance sensors like ultrasonics or lasers, as they are prone to interference.
// This sensor works well in a short term but error can build up.
class OTOS {
    private:
    public:
    QwiicOTOS sparkfun_otos;
    private:
    float total_y = 0;
    float previous_y = 0;
    public:
    void set_up(); // set up the sensor
    void set_pos(float x, float y, float rotation); // set the position
    bool working  = true;
    /* return position vector of robot */
    Vector get_posv();
    float get_heading();
};

#endif