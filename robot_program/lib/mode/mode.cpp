#include "mode.hpp"

PID linear_pid;
Vector find_closest_neutral_point(Vector pos) {
    std::vector<Vector> neutral_points = {
        Vector(0, 0),
        Vector(30, 0),
        Vector(-30, 0),
    };
    Vector closest_point = neutral_points[0];
    float closest_dist = (pos.relative_to(closest_point)).magnitude();
    for (Vector point : neutral_points) {
        float dist = (pos.relative_to(point)).magnitude();
        if (dist < closest_dist) {
            closest_dist = dist;
            closest_point = point;
        }
    }
    return closest_point;
}

// get robot rotation value to face a target angle
float Mode::get_rotation(float target_angle, float heading) {
    float rotation = target_angle - heading - M_PI_2;
    while (rotation > M_PI) rotation -= 2*M_PI;
    while (rotation < -M_PI) rotation += 2*M_PI;
    return rotation;
}

IndependentAttack::IndependentAttack(AimMode aim_mode) {
    this->aim_mode = aim_mode;
}

// kicks the ball straight. No goal targetting.
float IndependentAttack::calculate_move_angle_straight(float heading, float ball_angle, float ball_magnitude) {
    if (ball_magnitude < BALL_STRENGTH_LIMIT) {
        return ball_angle;
    }
    float lim_up = M_PI_2 + FORWARD_TOLERANCE;
    float lim_down = M_PI_2 - FORWARD_TOLERANCE;
    if (ball_angle > lim_down && ball_angle < lim_up) {
        // the ball is right in front of the bot
        return M_PI_2 + heading;
    }
    else if ((ball_angle > lim_up) || (ball_angle < -M_PI_2)) {
        // Serial.println("Turning right");
        return ball_angle + M_PI / 18 * 6; // turn right
    }
    else if ((ball_angle < lim_down)) {
        // Serial.println("Turning left");
        return ball_angle - M_PI / 18 * 6; // turn left
    }
}


// target the goal using camera
// if the goal is not detected, kick forwards
float IndependentAttack::calculate_move_angle_camera(float heading, float ball_angle, float ball_magnitude) {
    if (ball_magnitude < BALL_STRENGTH_LIMIT) {
        return ball_angle;
    }
    float lim_up = M_PI_2 + FORWARD_TOLERANCE + heading;
    float lim_down = M_PI_2 - FORWARD_TOLERANCE + heading;
    if (ball_angle > lim_down && ball_angle < lim_up) {
        // the ball is right in front of the bot
        return M_PI_2 + heading;
    }
    else if ((ball_angle > lim_up) || (ball_angle < -M_PI_2)) {
        // Serial.println("Turning right");
        return ball_angle + M_PI / 18 * 6; // turn right
    }
    else if ((ball_angle < lim_down)) {
        // Serial.println("Turning left");
        return ball_angle - M_PI / 18 * 6; // turn left
    }
}

float IndependentAttack::calculate_move_angle_otos(float heading, float ball_angle, float ball_magnitude, Vector goal_vec) {
    while (ball_angle >= PI) ball_angle -= 2 * PI;
    while (ball_angle <= -PI) ball_angle += 2 * PI;
    float angle_diff = PI / 2 - goal_vec.heading();
    if (ball_magnitude < BALL_STRENGTH_LIMIT) {
        // Serial.println("ball far");
        return ball_angle;
    }
    
    float low_bound = goal_vec.heading() - FORWARD_TOLERANCE;
    float high_bound = goal_vec.heading() + FORWARD_TOLERANCE;
    Serial.printf("low %.2f, high %.2f", low_bound * 180/M_PI, high_bound * 180/M_PI);
    if (ball_angle > low_bound && ball_angle < high_bound) {
        // Vector aim_vec = goal_vec;
        Serial.println("forward");
        // return aim_vec.heading(); // move forward
        return ball_angle;
    }
    else if ((ball_angle > high_bound) || (ball_angle < -M_PI_2 + angle_diff)) {
        Serial.println("Turning right");

        return ball_angle + PI / 18 * 8; // turn right
    }
    else if ((ball_angle < low_bound)) {
        Serial.println("Turning left");
        return ball_angle - PI / 18 * 8; // turn left
    }


    return 0.0;
}

float IndependentAttack::calculate_move_angle_camera_otos(Vector goal_vec, float heading, float ball_angle, float ball_magnitude) {return 0.0;};



OutputData IndependentAttack::update(BotData &self_data, BotData &other_data, float loop_time) {
    float mv_angle = M_PI_2;
    float rotation = 0;
    float speed = 100;
    float heading = self_data.heading;
    while (heading > M_PI) heading -= 2 * M_PI;
    while (heading < -M_PI) heading += 2 * M_PI;

    switch (this->aim_mode) {
        case STRAIGHT_MODE: {
            mv_angle = this->calculate_move_angle_straight(heading, self_data.ball_angle, self_data.ball_strength);
            rotation = -heading;
            break;
        }

        case OTOS_MODE: {
            Vector opp_goal_vector = opp_goal_pos_vector.relative_to(self_data.pos_vector);
            rotation = opp_goal_vector.heading() - heading - M_PI_2;

            // Serial.println("on my way to calculate mv angle otos");
            mv_angle = this->calculate_move_angle_otos(heading, self_data.ball_angle, self_data.ball_strength, opp_goal_vector);
            break;
        }
        case OTOS_REFLECTION_MODE: { // this is the same thing as OTOS_MODE except it aims into the reflection of the goal.
            Vector aim_vec = opp_goal_pos_vector;
            aim_vec.i -= 150;

            Vector opp_goal_vector = aim_vec.relative_to(self_data.pos_vector);
            rotation = aim_vec.heading() - heading - M_PI_2;
            mv_angle = this->calculate_move_angle_otos(heading, self_data.ball_angle, self_data.ball_strength, opp_goal_vector);
            break;
        }

        case CAMERA_MODE: {
            int pixel_diff = self_data.goal_x - 80;
            rotation = pixel_diff * -(M_PI / 160.0);
            if (self_data.goal_x == -1) {
                pixel_diff = 0;
                rotation = -heading;
            }
            mv_angle = this->calculate_move_angle_camera(heading, self_data.ball_angle, self_data.ball_strength);
            break;
        }

        case CAMERA_OTOS_MODE: {
            rotation = 0;
            mv_angle = 0;
            speed = 0;
            break;
        }
    };
    while (rotation > M_PI) rotation -= 2*M_PI;
    while (rotation < -M_PI) rotation += 2*M_PI;

    bool dribbler_on = false; // will do this later.

    return OutputData {.angle = mv_angle, .speed = speed, .rotation = rotation, .dribbler_on = dribbler_on};
}


