#ifndef _MODE_HPP_
#define _MODE_HPP_

#pragma once

#include <iostream>
#include <cmath>

#include "constants.h"
#include "bot_data.h"
#include "output_data.h"
#include "vector.hpp"
#include "pid.hpp"
#include <Arduino.h>
// Abstract parent class mode - bot data is passed to it and it returns the outputdata
class Mode {
    protected:
    float angle;
    float speed;
    float rotation;
    bool dribbler_on;

    public:
    float get_rotation(float target_angle, float heading);
    virtual OutputData update(BotData &self_data, BotData &other_data, float loop_time) = 0;
};

enum AimMode {
    STRAIGHT_MODE,
    OTOS_MODE,
    CAMERA_MODE,
    CAMERA_OTOS_MODE,
};

class IndependentAttack : public Mode {
    public:
    OutputData update(BotData &self_data, BotData &other_data, float loop_time);
    IndependentAttack(enum AimMode aim_mode);
    private:
    enum AimMode aim_mode;
    float calculate_move_angle_straight(float heading, float ball_angle, float ball_magnitude);
    float calculate_move_angle_otos(float heading, float ball_angle, float ball_magnitude, Vector current_pos);
    float calculate_move_angle_camera(float heading, float ball_angle, float ball_magnitude);
    float calculate_move_angle_camera_otos(Vector goal_vec, float heading, float ball_angle, float ball_magnitude);

};

// defender mode
class BetterDefend : public Mode {
    private:
    Vector target_posv;
    Vector target_vec;
    int status;
    float find_move_angle(Vector goal_vector, float ball_angle, float ball_strength);

    public:
    const int RETURNING = 0;
    const int DEFENDING = 1;
    BetterDefend();
    void reset();
    OutputData update(BotData &self_data, BotData &other_data, float loop_time);

    // outsiders can view status but can't modify it
    const int& get_status_code() const {
        return this->status;
    };
    // use by calling the function
};



#endif