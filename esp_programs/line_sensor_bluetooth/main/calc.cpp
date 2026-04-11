#include "calc.hpp"

MathVector::MathVector(float i, float j) {
    this->i = i;
    this->j = j;
}

float MathVector::magnitude() {
    return powf(powf(this->i, 2) + powf(this->j, 2), 0.5);
}
float MathVector::argument() {
    return atan2(this->j, this->i);
}
float MathVector::dot(MathVector vec) {
    return this->i * vec.i + this->j * vec.j;
}
float MathVector::scalar_proj_on(MathVector vec) {
    if (this->dot(vec) == 0) {
        return 0;
    }
    return this->dot(vec)/vec.dot(vec);
}
MathVector MathVectorFromArgument(float magnitude, float argument) {
    return MathVector(cos(argument) * magnitude, sin(argument) * magnitude);
}
std::array<bool, CHANNEL_NUM> find_white(int32_t sensor_values[CHANNEL_NUM], int32_t threshold) {
    std::array<bool, CHANNEL_NUM> output;
    for (uint8_t i = 0; i < CHANNEL_NUM; i++) {
        output[i] = sensor_values[i] < threshold;
    }
    return output;
}

std::array<float, 2> find_line(std::array<bool, CHANNEL_NUM> whites, std::array<MathVector, 16> sensor_vectors) {
    MathVector sum_vector(0.0, 0.0);
    for (uint8_t i = 0; i < CHANNEL_NUM; i++) {
        if (!whites[i]) {
            continue;
        }
        sum_vector.i += sensor_vectors[i].i;
        sum_vector.j += sensor_vectors[i].j;
    }
    if (sum_vector.i == 0 && sum_vector.j == 0) {
        return {0, 0};
    }
    float angle = sum_vector.argument();
    MathVector line_vector = MathVectorFromArgument(1, angle);

    std::vector<float> distances;
    for (uint8_t i = 0; i < 16; i++) {
        if (whites[i]) {
            float projection = sensor_vectors[i].scalar_proj_on(line_vector);
            distances.push_back(projection);
        }

    }
    float distance = std::accumulate(distances.begin(), distances.end(), 0.0f) / distances.size();
    return {(float)(angle), distance};
}