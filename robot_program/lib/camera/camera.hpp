#ifndef CAMERA_HPP
#define CAMERA_HPP
#pragma once

/*
We use the camera to identify the direction of the goal.
*/

#include <iostream>
#include <Arduino.h>
#include <cmath>

class Camera {
    public:
    int goal_x = -1;
    int goal_y = -1;

    bool read_success = false;

    // reading data from the camera
    bool read_serial(int* result, int num_ints);

    // update camera data
    // this must be called every loop
    void update(); 
};

#endif