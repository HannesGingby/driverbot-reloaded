#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <Servo.h>

// Define motor and servo pins, regular steering
//#define MOTOR_PIN_RIGHT_DIR 5    // D1 (GPIO 5)
//#define MOTOR_PIN_RIGHT_SPEED 0  // D3 (GPIO 0)
//#define SERVO_PIN 14             // D5 (GPIO 14)

// For tank steering
#define MOTOR_1_DIR_PIN 0    // D3
#define MOTOR_1_SPEED_PIN 5  // D1
#define MOTOR_2_DIR_PIN 2   // D4
#define MOTOR_2_SPEED_PIN 4  // D2


// #define SERVO_PIN 14             // D5 (GPIO 14)

// Declare the servo object used for steering
// extern Servo servo;

// Maximum motor speed constant
extern const int maxMotorSpeed;

// Keep track of steering state, cannot steer while going forward or backward
extern bool isSteering;

// Motor control functions
void drive(int speed, bool direction);
void steer(int speed, bool direction);
void stopMotors();
void stopSteering();

#endif // MOTOR_CONTROL_H
