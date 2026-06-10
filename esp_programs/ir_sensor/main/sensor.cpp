#include "sensor.hpp"

SensorRing::SensorRing() {
    for (uint8_t i = 1; i < 17; i++) {
        gpio_config_t config = {
            .pin_bit_mask = (1ULL << i),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&config);
    }
    this->sensor_vectors = {
        PolarVector{.a = 0.0f, .m = 0}, // 1
        PolarVector{.a = -1 * M_PI/8, .m = 0}, // 2
        PolarVector{.a = -2 * M_PI/8, .m = 0}, // 3
        PolarVector{.a = -3 * M_PI/8, .m = 0}, // 4
        PolarVector{.a = -4 * M_PI/8, .m = 0}, // 5
        PolarVector{.a = -5 * M_PI/8, .m = 0}, // 6
        PolarVector{.a = -6 * M_PI/8, .m = 0}, // 7
        PolarVector{.a = -7 * M_PI/8, .m = 0}, // 8
        PolarVector{.a = 8 * M_PI/8, .m = 0}, // 9
        PolarVector{.a = 7 * M_PI/8, .m = 0}, // 10
        PolarVector{.a = 6 * M_PI/8, .m = 0}, // 11
        PolarVector{.a = 5 * M_PI/8, .m = 0}, // 12 
        PolarVector{.a = 4 * M_PI/8, .m = 0}, // 13
        PolarVector{.a = 3 * M_PI/8, .m = 0}, // 14
        PolarVector{.a = 2 * M_PI/8, .m = 0}, // 15
        PolarVector{.a = 1 * M_PI/8, .m = 0}, // 16
    };
}

void SensorRing::update() {
    for (uint8_t i = 0; i < 16; i++) {
        this->sensor_vectors[i].m = 0;
    }

    for (uint8_t i = 0; i < 16; i++) {
        int64_t start_time = esp_timer_get_time();
        while (esp_timer_get_time() - start_time < 833*2) {
            int64_t time = esp_timer_get_time();
            this->sensor_vectors[i].m += !gpio_get_level((gpio_num_t)(16 - i));
            while (time == esp_timer_get_time());
        }
    }
}

void SensorRing::get_sorted_vectors() {
    this->sorted_indexes = {0, 1 ,2 ,3 ,4 ,5 ,6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    this->sorted_vectors = SortPolarVectors(this->sensor_vectors, &this->sorted_indexes);
}
PolarVector SensorRing::simpleRead() {
    return this->sorted_vectors[15];
}

// std::array<float, 2> SensorRing::advancedRead(std::array<std::array<float, 2>, 16> sensor_vectors, float *direction, float *strength) {
    
// }
// std::array<float, 2> SensorRing::advancedRead(std::array<std::array<float, 2>, 16> sensor_vectors, float *direction, float *strength) {
//     std::array<float, 2> sum_vector = {0, 0};
    
//     std::array<uint32_t, 16> sorted_readings = this->readings;
//     std::sort(sorted_readings.begin(), sorted_readings.end());

//     std::array<std::array<float, 2>, 16> sorted_sensor_vectors;
//     for (uint8_t i = 0; i < 16; i++) {
//         // Find the original index of the current sorted reading
//         auto it = std::find(this->readings.begin(), this->readings.end(), sorted_readings[i]);
//         size_t index = std::distance(this->readings.begin(), it);

//         // Use that index to get the corresponding sensor vector
//         sorted_sensor_vectors[i] = sensor_vectors[index];
//     }


//     for (uint8_t i = 0; i < 3; i++) {
//         sum_vector[0] += sorted_sensor_vectors[15-i][0]/* * sorted_readings[15-i]*/;
//         sum_vector[1] += sorted_sensor_vectors[15-i][1]/* * sorted_readings[15-i]*/;
//         // printf("%f, %f ", sorted_sensor_vectors[i][0], sorted_sensor_vectors[i]);
//     }
//     sum_vector[0] /= 3.0;
//     sum_vector[1] /= 3.0;
//     *direction = atan2(sum_vector[1], sum_vector[0]) * 180.0 / M_PI;
//     *strength = pow(pow(sum_vector[0], 2) + pow(sum_vector[1], 2), 0.5);
//     return sum_vector;
// }

std::array<float, 4> SensorRing::advancedRead() {
    ComponentVector sum_vector = ComponentVector();
    for (uint8_t i = 0; i < 16; i++) {
        sum_vector.i += cosf(this->sorted_vectors[15-i].a) * this->sorted_vectors[15-i].m;
        sum_vector.j += sinf(this->sorted_vectors[15-i].a) * this->sorted_vectors[15-i].m;
    }
    sum_vector.i /= 16;
    sum_vector.j /= 16;
    PolarVector ball_vector = {.a = atan2(sum_vector.j, sum_vector.i), .m = sqrtf(pow(sum_vector.i, 2) + pow(sum_vector.j, 2))};
    return {ball_vector.a, ball_vector.m, sum_vector.i, sum_vector.j};
}