#include "line_sensor.hpp"

// Read data from Line sensor using UART
bool LineSensor::read_serial(uint8_t* result) {
    const size_t max_buffer = 256;
    uint8_t buffer[max_buffer];
    int len = 0;
    if (Serial1.available() < TOTAL_BYTES) {
        return false;
    }
    // read all available bytes into a temp buffer
    while (Serial1.available() && len < max_buffer) {
        buffer[len++] = Serial1.read();
    }

    // find the last 'e' that has enough bytes after it
    int start_index = -1;
    for (int i = len - TOTAL_BYTES - 1; i >= 0; i--) {
        if (buffer[i] == 'e' && (i + 1 + TOTAL_BYTES <= len)) {
            start_index = i;
            break;
        }
    }

    if (start_index == -1) {
        // Serial.println("No full message found");
        return false;
    }

    // Step 3: Extract the float bytes
    const uint8_t* received_bytes = &buffer[start_index + 1];
    memcpy(result, received_bytes, TOTAL_BYTES);

    return true;
}

// send data for robot communication
void LineSensor::send_bot_data(BotData self_data) {
    uint8_t buffer[sizeof(BotData) + 1];
    buffer[0] = 'd';
    memcpy(&buffer[1], &self_data, sizeof(BotData));
    Serial1.write(buffer, sizeof(BotData) + 1);
}

void LineSensor::update() {
    uint8_t data[TOTAL_BYTES];

    this->read_success = this->read_serial(data);

    if (this->read_success) {
        memcpy(&this->angle, &data[0], sizeof(float));
        memcpy(&this->distance, &data[sizeof(float)], sizeof(float));

        memcpy(&this->other_data,
               &data[2 * sizeof(float)],
               sizeof(BotData));

        this->other_data_received = data[2 * sizeof(float) + sizeof(BotData)];
    }
}

// Calculates the true angle of line relative to the field not the robot (heading in radians)
void LineSensor::angle_correction(float heading) {
    if (this->read_success) this->angle = fmodf(this->angle + heading, 2*PI);
}

// Returns distance in mm
float LineSensor::get_distance() {
    return std::round(this->distance * 1000) / 1000;
}

// Returns angle in radians
float LineSensor::get_angle() {
    return std::round(this->angle * 1000) / 1000;
}