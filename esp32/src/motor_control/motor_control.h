#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Define pins
#define MOTOR_AIN1_PIN 2
#define MOTOR_AIN2_PIN 3
#define MOTOR_BIN1_PIN 20
#define MOTOR_BIN2_PIN 23

#define MOTOR_A_PWM_PIN 0
#define MOTOR_B_PWM_PIN 1

// Maximum motor speed constant
extern const int maxMotorSpeed;

// Keep track of steering state, cannot steer while going forward or backward
extern bool isSteering;

// Motor control functions
void drive(int speed, bool direction);
void steer(int speed1, int speed2, bool direction);
int clampSpeed(int speed);
void stopMotors();

#endif // MOTOR_CONTROL_H
