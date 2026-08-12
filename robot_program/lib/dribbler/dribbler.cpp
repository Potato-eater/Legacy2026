#include "dribbler.hpp"

DribblerMotor::DribblerMotor(int dir_pin, int pwm_pin) {
    this->DIR_PIN = dir_pin;
    this->PWM_PIN = pwm_pin;
}

void DribblerMotor::run(float speed) {
    int pwmSpeed = speed / 100 * 255; // convert percentage into 0-255
    digitalWrite(this->DIR_PIN, HIGH); // set direction to positive
    analogWrite(this->PWM_PIN, pwmSpeed);
}

void DribblerMotor::run_reverse(float speed) {
    int pwmSpeed = speed / 100 * 255; // convert percentage to 0-255
    digitalWrite(this->DIR_PIN, LOW); // set direction to negative
    analogWrite(this->PWM_PIN, pwmSpeed);

}

void DribblerMotor::stop() {
    digitalWrite(this->DIR_PIN, LOW);
    analogWrite(this->PWM_PIN, 0); // turns off the dribbler.
}