#ifndef _BOT_DATA_HPP_
#define _BOT_DATA_HPP_

#pragma once

#include <iostream>
#include "vector.hpp"

// A struct containing robot data
struct BotData {
    float heading; // the angle that the robot is facing
    Vector pos_vector; // the (i, j) location of where the robot is currently. (0,0) being the centre of the field.
    float ball_strength; // the strength detected from the IR ball
    float ball_angle; // the angle of the ball, relative to the robot's location
    Vector line_vector; // the direction and distance to the line (if detected)
    Vector velocity; // how fast the robot is going. NOT currently used.
    int goal_x; // the x location of the goal detected from the camera
};

#endif