#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Define pins
#define MOTOR_1_DIR_PIN 0    // D3
#define MOTOR_1_SPEED_PIN 5  // D1
#define MOTOR_2_DIR_PIN 2   // D4
#define MOTOR_2_SPEED_PIN 4  // D2

// Maximum motor speed constant
extern const int maxMotorSpeed;

// Keep track of steering state, cannot steer while going forward or backward
extern bool isSteering;

// Motor control functions
void drive(int speed, bool direction);
void steer(int speed, bool direction);
void stopMotors();

#endif // MOTOR_CONTROL_H
