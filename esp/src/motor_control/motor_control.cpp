#include "motor_control.h"

const int maxMotorSpeed = 1024;
bool isSteering = false;


void drive(int speed, bool direction) {
    // direction = 1, forward
    // direction = 0, backward

    if (!isSteering) {
        if (0 <= speed && speed <= maxMotorSpeed) {
            Serial.print("Running motors with speed: ");
            Serial.println(speed);
            Serial.print("and direction: ");
            Serial.println(direction);

            analogWrite(MOTOR_1_SPEED_PIN, speed);
            digitalWrite(MOTOR_1_DIR_PIN, direction);

            analogWrite(MOTOR_2_SPEED_PIN, speed);
            digitalWrite(MOTOR_2_DIR_PIN, direction);
        } else {
            Serial.println("Not running motors, 'speed' is out of range");
        }
    }
}


void steer(int speed, bool direction) {
    // direction = 1, right
    // direction = 0, left

    isSteering = true;

    Serial.print("Steering in direction: ");
    Serial.println(direction);
    Serial.print("with speed: ");
    Serial.println(speed);

    analogWrite(MOTOR_1_SPEED_PIN, speed);
    digitalWrite(MOTOR_1_DIR_PIN, !direction);

    analogWrite(MOTOR_2_SPEED_PIN, speed);
    digitalWrite(MOTOR_2_DIR_PIN, direction);
}


void stopMotors() {
    isSteering = false;

    Serial.print("Stopping motors");

    analogWrite(MOTOR_1_SPEED_PIN, 0);
    analogWrite(MOTOR_2_SPEED_PIN, 0);
    digitalWrite(MOTOR_1_DIR_PIN, 0);
    digitalWrite(MOTOR_2_DIR_PIN, 0);
}
