#ifndef _OUTPUT_DATA_HPP_
#define _OUTPUT_DATA_HPP_

#pragma once

#include <iostream>

// A struct for mode outputs
struct OutputData {
    float angle; // which way the robot should be going
    float speed; // how fast the robot should go
    float rotation; // how much the robot needs to rotate
    bool dribbler_on; // if the dribbler should be on currently.
};

#endif