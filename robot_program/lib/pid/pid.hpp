#ifndef _PID_HPP_
#define _PID_HPP_

#pragma once

#include "vector.hpp"

// our own implementation of a PID controller. used in going to a position accurately.
// look here for a good explainaiton: https://en.wikipedia.org/wiki/PID_controller
class PID {
    public:
    const float PROPORTIONAL_CONSTANT = 2.0;
    const float INTEGRAL_CONSTANT = 0.0005;
    const float DERIVETIVE_CONSTANT = 0.1;

    float MINIMUM_PID_SPEED = 30;
    double previous_error, integral, derivitive;
    float angle, speed;
    Vector get_movement(Vector pos, Vector target_pos, float max_speed, double dt);
  
    private:
    double compute(double error, double dt);
    float min(float var1, float var2);
    float max(float var1, float var2);
};

#endif