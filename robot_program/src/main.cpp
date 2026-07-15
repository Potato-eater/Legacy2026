// main code for the robots for team Legacy, 2026. We are team from Rossmoyne Senior High School.

// include all the libraries.
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <cmath>
#include <iostream>
#include <SoftwareSerial.h>

#include "position_system.hpp"
#include "pins.h"
#include "vector.hpp"
#include "motor_controller.hpp"
#include "ir_sensor.hpp"
#include "line_sensor.hpp"
#include "dribbler.hpp"
#include "mode.hpp"
#include "pid.hpp"
#include "camera.hpp"
#include "compute.hpp"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// pre-defining some functions.
bool set_robot_pos();
void print_botdata(BotData &bot_data, String message);

// checks if the robot is on the line
void check_line(float heading, Vector line_vector, PositionSystem *pos_sys, float *move_angle) {
  if (line_vector.magnitude() != 0) {
    *move_angle = line_vector.heading() + M_PI;
    // Vector current_pos = pos_sys->get_posv();

    // float tolerance = M_PI / 8.0;
    // if (line_vector.heading() < tolerance && line_vector.heading() > -tolerance) {
    //   Vector new_pos = Vector(50.0, current_pos.j);
    //   pos_sys->set_pos(new_pos, heading * 180.0 / M_PI);
    // }
    // if (line_vector.heading() < M_3PI_4 && line_vector.heading() > M_PI_4) {
    //   Vector new_pos = Vector(current_pos.i, 91.5);
    //   pos_sys->set_pos(new_pos, -heading * 180.0 / M_PI);
    // }
    // if (line_vector.heading() < -M_PI + tolerance || line_vector.heading() > M_PI - tolerance) {
    //   Vector new_pos = Vector(-50.0, current_pos.j);
    //   pos_sys->set_pos(new_pos, heading * 180.0 / M_PI);
    // }
    // if (line_vector.heading() < -M_PI_4 && line_vector.heading() > -M_3PI_4) {
    //   Vector new_pos = Vector(current_pos.i, -91.5);
    //   pos_sys->set_pos(new_pos, -heading * 180.0 / M_PI);
    // }
  }
}


// init all the objects and variables.
// the arduino framework uses a setup and loop function,
// so we have to define everything as global if we need to use them across iterations.
IRSensor ir_sensor;
LineSensor line_sensor;
Camera camera;
PositionSystem pos_sys;
DribblerMotor dribbler(DR_DIR, DR_PWM);
MotorController motor_ctrl;

bool robot_start = false; // thie variable defines if the robot should move.

bool button_pressed = false; 
bool prev_robot_state = false;

float heading_offset = 0;
// DribblerMotor dribbler(DR_DIR, DR_PWM);
Adafruit_SSD1306 display(128, 32, &Wire, -1);

// Modes
// using polymorphism, we can have different modes, that all returns the same datatype.

IndependentAttack independent_attack(AimMode::CAMERA_MODE);
BetterDefend better_defend;
// OneRobot one_robot_mode;
// BetterDefend better_defend_mode;
// uint8_t previous_mode = 0;


int loop_time = 0;


float time_start = millis();
float time_end = millis();



void setup() {
  // put your setup code here, to run once:

  // initialising UART communication and USB debugging.
  Serial.begin(921600);
  Serial1.begin(921600); // Line Sensor
  Serial6.begin(921600); // IR Sensor
  Serial3.begin(115200); // Camera


  // initialising GPIO
  pinMode(DEBUG_LED, OUTPUT);

  // Drive motors
  pinMode(TL_PWM, OUTPUT); pinMode(TR_PWM, OUTPUT); pinMode(BL_PWM, OUTPUT); pinMode(BR_PWM, OUTPUT);
  analogWriteFrequency(TL_PWM, 20000); analogWriteFrequency(TR_PWM, 20000);
  analogWriteFrequency(BL_PWM, 20000); analogWriteFrequency(BR_PWM, 20000);
  pinMode(TL_DIR, OUTPUT); pinMode(TR_DIR, OUTPUT); pinMode(BL_DIR, OUTPUT); pinMode(BR_DIR, OUTPUT);

  // make sure all the driver motors are stopped.
  motor_ctrl.stop_motors();

  // Start buttons
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);
  pinMode(BTN_5, INPUT_PULLUP);

  pinMode(BTN_RUN, INPUT_PULLUP);

  // Dribbler pins
  pinMode(DR_PWM, OUTPUT);
  pinMode(DR_DIR, OUTPUT);

  pos_sys.setup(); // initialise the bno055 gyro and sparkfun OTOS

  // Display Setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(2);     
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);  

  // Displaying Ready and compile date/time
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);   
  display.println("Ready");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(__DATE__); display.print(" "); display.println(__TIME__);
  display.display();

  Serial.println("Awaiting button press");

  // pwmTimer.begin(pwmISR, 10);
  // analogWrite(TL_PWM, 256);
  // analogWrite(BL_PWM, 256);
  // analogWrite(BR_PWM, 256);
  better_defend.reset();
}


void loop() {
  // Serial.println("hello world");
  set_robot_pos();
  // button_pressed = !digitalReadFast(BTN_1) | !digitalReadFast(BTN_2) | !digitalReadFast(BTN_3) | !digitalReadFast(BTN_4) | !digitalReadFast(BTN_5);
  robot_start = !digitalReadFast(BTN_RUN);
  if (robot_start && prev_robot_state == false) {
  //   robot_start = true;
    
    heading_offset = pos_sys.get_heading();
  }
  // else if (button_pressed && robot_start == true && prev_button_state == false) {
  //   robot_start = false;
  // }

  line_sensor.update();
  ir_sensor.update();
  pos_sys.update();
  camera.update();
  float heading = pos_sys.get_heading() - heading_offset;
  while (heading >= PI) heading -= 2 * PI;
  while (heading <= -PI) heading += 2 * PI;


  ir_sensor.angle_correction(heading);
  line_sensor.angle_correction(heading);


  float ir_angle = ir_sensor.get_angle();
  while (ir_angle < -PI) ir_angle += 2*PI;
  while (ir_angle > PI) ir_angle -= 2*PI;

  if (ir_sensor.get_magnitude() == 0) {
    ir_angle = 0;
  }
  BotData self_data = {
    .heading = heading,
    .pos_vector = pos_sys.get_posv(),
    .ball_strength = ir_sensor.get_magnitude(),
    .ball_angle = ir_angle,
    .line_vector = Vector::from_heading(line_sensor.get_angle(), line_sensor.get_distance()),
    .velocity = Vector(0.0, 0.0),
    .goal_x = camera.goal_x,
  };

  line_sensor.send_bot_data(self_data);
  BotData other_data = line_sensor.other_data;

  // OutputData output = independent_attack.update(self_data, other_data, 0.0);
  OutputData output = independent_attack.update(self_data, other_data, 0.0);
  // if (self_data.ball_strength > other_data.ball_strength) {
  //   output = independent_attack.update(self_data, other_data, 0.0);
  // }
  // else {
  //   output = better_defend.update(self_data, other_data, 0.0);
  // }

  // if (self_data.line_vector.magnitude() != 0) {
  //   output.angle = self_data.line_vector.heading() + M_PI;
  // }
  check_line(self_data.heading, self_data.line_vector, &pos_sys, &output.angle);
  
  if (!robot_start || self_data.ball_strength == 0) {
    // speed = 0;
    motor_ctrl.stop_motors();
  }
  else {

    motor_ctrl.run_motors(output.speed, output.angle, output.rotation);
  }
  Vector g_vec = opp_goal_pos_vector.relative_to(self_data.pos_vector);
  // Serial.printf("%.2f\n", g_vec.heading() * 180.0 / M_PI);
  Serial.printf("%d\n", line_sensor.other_data_received);










  prev_robot_state = robot_start;
  dribbler.run();
  digitalWrite(DEBUG_LED, HIGH);
}




bool set_robot_pos() {
  if (!digitalRead(BTN_1)) {
    pos_sys.set_pos(Vector(0, -11.5), 0); // set position of otos (kick off)
    return true;
  }
  if (!digitalRead(BTN_2)) {
    pos_sys.set_pos(Vector(-41, -69.5), 0); // set position of otos
    return true;
  }
  if (!digitalRead(BTN_3)) {
    pos_sys.set_pos(Vector(0, -64.0), 0); // set position of otos (center front)
    return true;
  }
  if (!digitalRead(BTN_4)) {
    pos_sys.set_pos(Vector(41, -69.5), 0); // set position of otos
    return true;
  }
  if (!digitalRead(BTN_5)) {
    pos_sys.set_pos(Vector(0, -87.5), 0); // set position of otos (center back)
    return true;
  }
  return false;
}

void print_botdata(BotData &bot_data, String message) {
  Serial.print(message + " | heading:");
  Serial.print(bot_data.heading*180/PI);
  Serial.print(" posv:<");
  Serial.print(bot_data.pos_vector.i);
  Serial.print(",");
  Serial.print(bot_data.pos_vector.j);
  Serial.print("> strength:");
  Serial.print(bot_data.ball_strength);
  Serial.print(" IR angle:");
  Serial.print(bot_data.ball_angle*180/PI);
  Serial.print(" line angle:");
  Serial.print(bot_data.line_vector.heading()*180/PI);
  Serial.print(" line distance:");
  Serial.println(bot_data.line_vector.magnitude());
}