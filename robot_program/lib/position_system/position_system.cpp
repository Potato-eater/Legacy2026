#include "position_system.hpp"

PositionSystem::PositionSystem() {
    this->bno = Adafruit_BNO055(55);
    this->bno_ok = false;


    
    this->opp_goal_posv = Vector(0, 91.5);
    this->own_goal_posv = Vector(0, -91.5);
    this->posv = Vector(0, 0);
    this->heading = 0;
}

bool PositionSystem::check_bno_ok() {
    return this->bno_ok;
}

bool PositionSystem::check_otos_ok() {
    return this->otos.working;
}

Vector PositionSystem::get_posv() {
    return this->posv;
}

// returns heading in radians
float PositionSystem::get_heading() {
    return fmodf(this->heading, 2*M_PI);
}

Vector PositionSystem::get_relative_to(Vector other_posv) {
    return Vector(other_posv.i-this->posv.i, other_posv.j-this->posv.j);
}

Vector PositionSystem::get_opp_goal_vec() {
    return this->get_relative_to(this->opp_goal_posv);
}
Vector PositionSystem::get_own_goal_vec() {
    return this->get_relative_to(this->own_goal_posv);
}

void PositionSystem::set_pos(Vector posv, float heading) {
    this->otos.set_pos(posv.i, posv.j, heading);
}

void PositionSystem::setup() {
    this->bno_ok = this->bno.begin(OPERATION_MODE_IMUPLUS);
    this->bno.setExtCrystalUse(true);
    this->otos.set_up();
}

void PositionSystem::update() {
    // get tilt from BNO or get tilt from parameter
    if (this->bno_ok) {
        sensors_event_t event;
        this->bno.getEvent(&event);
        this->heading = (360-event.orientation.x)*PI/180;
    }

    else {
        this->heading = this->otos.get_heading();
    }


    this->posv = this->otos.get_posv();
    
}

bool PositionSystem::within_opp_goal_range(Vector pos_vector) {
    if (pos_vector.j > 0 && pos_vector.i < GOAL_WIDTH/2 && pos_vector.i > -GOAL_WIDTH/2) {
        return true;
    }
    return false;
}

bool PositionSystem::within_own_goal_range(Vector pos_vector) {
    if (pos_vector.j < 0 && pos_vector.i < GOAL_WIDTH/2 && pos_vector.i > -GOAL_WIDTH/2) {
        return true;
    }
    return false;
}