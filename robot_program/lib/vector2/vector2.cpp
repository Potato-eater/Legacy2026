#include <iostream>

#include <cmath>
#include <vector>
#include <array>
#include "vector2.hpp"


CompVec::CompVec(float i, float j) {
    this->i = i;
    this->j = j;
}
float CompVec::get_magnitude() {
    return hypot(this->i, this->j);
}
float CompVec::get_i() {
    return this->i;
}
float CompVec::get_j() {
    return this->j;
}
float CompVec::get_dir() {
    return atan2(this->j, this->i);
}
void CompVec::get_unit(float *uniti, float *unitj) {
    float magnitude = this->get_magnitude();
    *uniti = this->i / magnitude;
    *unitj = this->j / magnitude;
}



PolVec::PolVec(float magnitude, float dir){
    this->magnitude = magnitude;
    this->dir = dir;
}
float PolVec::get_magnitude(){
    return this->magnitude;
}
float PolVec::get_i(){
    return (cos(this->dir) * this->magnitude);
}
float PolVec::get_j(){
    return (sin(this->dir) * this->magnitude);
}
float PolVec::get_dir(){
    return this->dir;
}
void PolVec::get_unit(float *uniti, float *unitj){
    *uniti = cos(this->dir);
    *unitj = sin(this->dir);
}


CompVec calc_ball_vector(CompVec Robot1, CompVec Robot2, PolVec IR1, PolVec IR2) {

    float k; float Ai = Robot1.get_i(); float Aj = Robot1.get_j(); float Bi = Robot2.get_i(); float Bj = Robot2.get_j(); float Ui = 0; float Uj = 0; float Wi = 0; float Wj = 0; IR1.get_unit(&Ui, &Uj); IR2.get_unit(&Wi, &Wj);
    float denominator = Wi * Uj - Wj * Ui;
    if (fabs(denominator) < 1e-6) {
        return CompVec(0.0, 0.0); 
    }
    k = (Bj * Ui - Aj * Ui - (Bi - Ai) * Uj) / denominator;

    CompVec ball_vector  =  CompVec((Bi + k * (Wi)), (Bj + k * (Wj)));    
    return ball_vector;
}
float return_to_goal(CompVec Robot1, CompVec own_goal_pos_vector) {

    float Ai = Robot1.get_i();
    float Aj = Robot1.get_j();
    float goal_i = own_goal_pos_vector.get_i();
    float goal_j = own_goal_pos_vector.get_j();

    CompVec robot_to_goal = CompVec((goal_i - Ai), (goal_j - Aj));

    if (robot_to_goal.get_i() == 0 && robot_to_goal.get_j() == 0) {
        return 0.0;
    }

    return robot_to_goal.get_dir();
} 