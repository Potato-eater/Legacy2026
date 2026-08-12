#include "mode.hpp"

PID linear_pid;
Vector find_closest_neutral_point(Vector pos) {
    std::vector<Vector> neutral_points = {
        Vector(0, -25.0),
        Vector(35, -25.0),
        Vector(-35, -25.0), 
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
        return ball_angle + M_PI / 18.0 * 7.5; // turn right
    }
    else if ((ball_angle < lim_down)) {
        // Serial.println("Turning left");
        return ball_angle - M_PI / 18.0 * 7.5; // turn left
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
	        const Vector goal_left(-22.5, 91.5);
	        const Vector goal_right(22.5, 91.5);

	        float heading_to_left = goal_left.relative_to(self_data.pos_vector).heading() - (M_PI / 18);
	        float heading_to_right = goal_right.relative_to(self_data.pos_vector).heading() + (M_PI / 18);
	        int pixel_diff = self_data.goal_x - 80;
            rotation = pixel_diff * -(M_PI / 160.0); 
            float relative_camera_angle = rotation + self_data.heading;
	
	        if (self_data.goal_x == -1) {
                pixel_diff = 0;
		        Vector opp_goal_vector = opp_goal_pos_vector.relative_to(self_data.pos_vector);
                rotation = opp_goal_vector.heading() - heading - M_PI_2;
                mv_angle = this->calculate_move_angle_otos(heading, self_data.ball_angle, self_data.ball_strength, opp_goal_vector);
                break;
            }    
            else if (heading_to_left >= relative_camera_angle && relative_camera_angle >= heading_to_right) {
                mv_angle = this->calculate_move_angle_camera(heading, self_data.ball_angle, self_data.ball_strength); 
                break;
	        }
            else {
                Vector opp_goal_vector = opp_goal_pos_vector.relative_to(self_data.pos_vector);
                rotation = opp_goal_vector.heading() - heading - M_PI_2;
                mv_angle = this->calculate_move_angle_otos(heading, self_data.ball_angle, self_data.ball_strength, opp_goal_vector);
                break;
		}
break;
}

        
    };
    if (self_data.ball_strength == 0) {
        Vector mv_vec = find_closest_neutral_point(self_data.pos_vector);
        mv_angle = mv_vec.relative_to(self_data.pos_vector).heading();
        rotation = -heading;
        speed = 80;
        dribbler_on = false;
        
    }
    while (rotation > M_PI) rotation -= 2*M_PI;
    while (rotation < -M_PI) rotation += 2*M_PI;

    bool dribbler_on = false; // will do this later.

    return OutputData {.angle = mv_angle, .speed = speed, .rotation = rotation, .dribbler_on = dribbler_on};
}

BetterDefend::BetterDefend() {
    this->status = 0;
    this->target_posv = Vector(0, 0);
    this->target_vec = Vector(0, 0);
}

void BetterDefend::reset() {
    this->status = this->RETURNING;
}

OutputData BetterDefend::update(BotData &self_data, BotData &other_data, float loop_time) {
    // if in goal square and sees the ball, start defending
    if (self_data.pos_vector.i > -GOAL_WIDTH/2 && self_data.pos_vector.i < GOAL_WIDTH/2 && self_data.pos_vector.j <= -65 && self_data.ball_strength != 0) {
        this->status = this->DEFENDING;
    }
    else {
        this->status = this->RETURNING;
    }

    Vector goal_vec = Vector(0, DEFEND_CENTRE_Y).relative_to(self_data.pos_vector);

    // if defending go on the semi-circle
    if (this->status == this->DEFENDING) {
        // limit rotation
        if (self_data.ball_angle > 3*PI/2) {
            this->rotation = this->get_rotation(0, self_data.heading);
        }
        else if (self_data.ball_angle > PI) {
            this->rotation = this->get_rotation(PI, self_data.heading);
        }
        else {
            this->rotation = this->get_rotation(self_data.ball_angle, self_data.heading);
        }

        // ACTUAL magical math happening here. if it works, dont touch it.
        // the robot goes in a semi circle around the goal to defend.
        Vector ball_vector = Vector::from_heading(self_data.ball_angle, DEFEND_OFFSET+DEFEND_Y);
        this->target_vec = Vector(goal_vec.i+ball_vector.i, goal_vec.j+ball_vector.j);
        
        this->angle = target_vec.heading();
    }
    // if returning go back to goal while avoiding the ball
    else if (this->status == this->RETURNING) {
        this->rotation = this->get_rotation(PI/2, self_data.heading);
        this->target_vec = Vector(goal_vec.i, goal_vec.j+15);
        this->angle = target_vec.heading();

        // difference in angle between ball angle and goal target vector
        float angle_diff = self_data.ball_angle - target_vec.heading();
        while (angle_diff > PI) angle_diff -= 2*PI;
        while (angle_diff < -PI) angle_diff += 2*PI;

        // if ball close and ball in the way off goal
        if (self_data.ball_strength >= 30 && angle_diff <= PI/2) {
            if (angle_diff != 0) this->angle += (abs(angle_diff)/angle_diff)*-PI/18*6;
            else this->angle += PI/18*6;
        }
        // if angle diff positive then ball is on the right therefore -60 degrees
        // if angle diff negative then ball is on the left therefore +60 degrees
    }

    // PID movement vector
    // Vector movement = linear_pid.get_movement(self_data.pos_vector, this->target_posv, MAX_SPEED, loop_time);
    // this->angle = movement.heading();
    // this->speed = movement.magnitude();

    this->dribbler_on = false;
    this->speed = 100;
    if (self_data.line_vector.magnitude() != 0) {
        // if in goal square dont bounce back from line
        if (self_data.pos_vector.i <= 22.5 && self_data.pos_vector.i >= -22.5) this->angle = Vector(this->target_vec.i, 0).heading();
        else this->angle = self_data.line_vector.heading() + PI;
    }

    return OutputData { .angle=this->angle, .speed=this->speed, .rotation=this->rotation, .dribbler_on=this->dribbler_on };
}
