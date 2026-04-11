#include "calc.hpp"

ComponentVector::ComponentVector(float i, float j) {
    this->i = i;
    this->j = j;
}

float ComponentVector::magnitude() {
    return powf(powf(this->i, 2) + powf(this->j, 2), 0.5);
}
float ComponentVector::argument() {
    return atan2(this->j, this->i);
}
float ComponentVector::dot(ComponentVector vec) {
    return this->i * vec.i + this->j * vec.j;
}
float ComponentVector::scalar_proj_on(ComponentVector vec) {
    if (this->dot(vec) == 0) {
        return 0;
    }
    return this->dot(vec)/vec.dot(vec);
}
ComponentVector ComponentVectorFromArgument(float magnitude, float argument) {
    return ComponentVector(cos(argument) * magnitude, sin(argument) * magnitude);
}



std::array<PolarVector, 16> SortPolarVectors(std::array<PolarVector, 16> arr, std::array<uint8_t, 16> *indexes) {
    std::array<PolarVector, 16> sorting_array = arr;
    // std::array<uint8_t, 16> indexes;
    for (uint8_t i = 0; i < 16; i++) {
        (*indexes)[i] = i;
    }
    bool swapped;
    do {
        swapped = false;
        for (uint8_t i = 0; i < 15; i++) {  // Only go to 15, since we're accessing i+1
            if (sorting_array[i].m > sorting_array[i + 1].m) {
                std::swap(sorting_array[i], sorting_array[i + 1]);
                std::swap((*indexes)[i], (*indexes)[i+1]);
                swapped = true;
            }
        }
    } while (swapped);  // Continue until no swaps are made

    return sorting_array;
}
