#ifndef CALC_HPP
#define CALC_HPP
#include <math.h>
#include <iostream>
#include <array>
#include <vector>
#include <numeric>

// We use Vectors to calculate the strength and direction of the ball.

class ComponentVector {
    // defining the Component Vector class, with i and j as the attributes and other mathematical methods.
    public:
    float i;
    float j;

    ComponentVector(float i = 0.0, float j = 0.0); // constructor
    float magnitude(); // gets magnitude of the result.
    float argument(); // gets angle from the x axis.
    float dot(ComponentVector vec); // calculates dot product with another vecotr
    float scalar_proj_on(ComponentVector vec); // calculates scalar projection on another vector
    
};
ComponentVector ComponentVectorFromArgument(float magnitude, float argument); // constructor, for polar vector.
struct PolarVector {
    float a;
    float m;
};

// this function would sort each polar vector based on their magnitude (sensor strength)
// allows us to only account for strongest sensors to avoid interference.
std::array<PolarVector, 16> SortPolarVectors(std::array<PolarVector, 16> arr, std::array<uint8_t, 16> *indexes);

#endif