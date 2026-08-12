#include "camera.hpp"



bool Camera::read_serial(int* result, int num_ints) {
    /*
    works by copying every byte received into a tempory buffer, and then finding the latest set of complete data.
    */

    const size_t total_bytes = num_ints * sizeof(int); // calculating the number of bytes expected
    const size_t max_buffer = 256; // defining buffer size
    uint8_t buffer[max_buffer]; // initialising the buffer
    int len = 0;
    if (Serial3.available() < total_bytes + 1) { // stop reading if there is not enough bytes available.
        return false;
    }
    // read all available bytes into a temp buffer
    while (Serial3.available() && len < max_buffer) {
        buffer[len++] = Serial3.read();
    }
    // find the last 'e' that has enough bytes after it
    int start_index = -1;
    for (int i = len - total_bytes - 1; i >= 0; i--) {
        if (buffer[i] == 'e' && (i + 1 + total_bytes <= len)) {
            start_index = i;
            break;
        }
    }

    if (start_index == -1) { // if the header byte is not found
        Serial.println("No full message found");
        return false;
    }

    // Extract the int bytes
    const uint8_t* int_bytes = &buffer[start_index + 1];
    memcpy(result, int_bytes, total_bytes); // copy the data across to the output.
    // Serial.println("message received");
    return true;
}

void Camera::update() {
    int data[4];
    this->read_success = this->read_serial(data, 2);
    if (this->read_success) {
        this->goal_x = data[0]; // assign each byte to their correct locations.
        this->goal_y = data[1];
    }
}