#ifndef CALC_HPP
#define CALC_HPP
#include <math.h>
#include <iostream>
#include <array>
#include <vector>
#include <numeric>
class ComponentVector {
    public:
    float i;
    float j;

    ComponentVector(float i = 0.0, float j = 0.0);
    float magnitude();
    float argument();
    float dot(ComponentVector vec);
    float scalar_proj_on(ComponentVector vec);
    
};
ComponentVector ComponentVectorFromArgument(float magnitude, float argument);
struct PolarVector {
    float a;
    float m;
};
std::array<PolarVector, 16> SortPolarVectors(std::array<PolarVector, 16> arr, std::array<uint8_t, 16> *indexes);

#endif