#include <iostream>
#include <Adafruit_BNO055.h>

#include "otos.hpp"
#include "constants.h"
#include <cmath>
#include <vector>
#include <array>

#include "constants.h"

class Vector2 {
    public:
    virtual float get_magnitude();
    virtual float get_i();
    virtual float get_j();
    virtual float get_dir();
    virtual void get_unit(float *uniti, float *unitj);
};

class CompVec : public Vector2 {

    float i;
    float j;

    public:
    float get_magnitude();
    float get_i();
    float get_j();
    float get_dir();
    void get_unit(float *uniti, float *unitj);
    CompVec(float i, float j);
};
class PolVec : public Vector2 {
    float magnitude;
    float dir;
    public:
    float get_magnitude();
    float get_i();
    float get_j();
    float get_dir();
    void get_unit(float *uniti, float *unitj);
    PolVec(float magnitude, float dir);
};

CompVec calc_ball_vector(CompVec Robot1, CompVec Robot2, PolVec IR1, PolVec IR2);
