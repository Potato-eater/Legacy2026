#include <math.h>
#include "constants.hpp"
#include <iostream>
#include <array>
#include <vector>
#include <numeric>

class MathVector {
    public:
    float i;
    float j;

    MathVector(float i = 0.0, float j = 0.0);
    float magnitude();
    float argument();
    float dot(MathVector vec);
    float scalar_proj_on(MathVector vec);
    
};
MathVector MathVectorFromArgument(float magnitude, float argument);


std::array<bool, CHANNEL_NUM> find_white(int32_t sensor_values[CHANNEL_NUM], int32_t threshold);

std::array<float, 2> find_line(std::array<bool, CHANNEL_NUM> whites, std::array<MathVector, 16> sensor_vectors);