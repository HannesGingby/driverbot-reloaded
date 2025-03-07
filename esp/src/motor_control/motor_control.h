#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <Servo.h>

// Define motor and servo pins
#define MOTOR_PIN_RIGHT_DIR 0    // D3 (GPIO 0)
#define MOTOR_PIN_RIGHT_SPEED 2  // D4 (GPIO 2)
#define SERVO_PIN 14             // D5 (GPIO 14)

// Declare the servo object used for steering
extern Servo servo;

// Maximum motor speed constant
extern const int maxMotorSpeed;

// Motor control functions
void driveForward(int speed);
void driveBackward(int speed);
void steerRight(int angle, int speed);
void steerLeft(int angle, int speed);
void stopDriving();
void resetSteering();

#endif // MOTOR_CONTROL_H
