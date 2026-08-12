#include "motor.hpp"

Motor::Motor(int pwm_pin, int dir_pin) { // initialising
  this->PWM_PIN = pwm_pin;
  this->DIR_PIN = dir_pin;
}

void Motor::run(float speed) {
  // get pwm from speed

  // the motor's speed is inversely proportional to the pwm value. 0 means 100% speed, 255 means 0% speed (stopping)
  int pwmSpeed = (100 - abs(speed)) / 100 * 255;
  if (speed > 0) {
    digitalWrite(this->DIR_PIN, LOW); // switch direction
  }
  else {
    digitalWrite(this->DIR_PIN, HIGH);
  }

  analogWrite(this->PWM_PIN, pwmSpeed);
}

void Motor::stop() {
  analogWrite(this->PWM_PIN, 255);
}