#include "motor_control.h"

Servo servo;
const int maxMotorSpeed = 255;

void driveForward(int speed) {
    Serial.print("Driving forward with the speed of ");
    Serial.println(speed);

    analogWrite(MOTOR_PIN_RIGHT_SPEED, 1);
    digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);

    // digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);
    // analogWrite(MOTOR_PIN_RIGHT_SPEED, speed);
}

void driveBackward(int speed) {
    Serial.print("Driving backward with the speed of ");
    Serial.println(speed);

    analogWrite(MOTOR_PIN_RIGHT_SPEED, 1);
    digitalWrite(MOTOR_PIN_RIGHT_DIR, 0);
    
    //digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);
    //analogWrite(MOTOR_PIN_RIGHT_SPEED, speed);
}

void steerRight(int angle, int speed) {
    Serial.print("Steering right with ");
    Serial.print(angle);
    Serial.println("° angle");
    // Directly set the servo angle
    servo.write(90 - (angle - 3));
}

void steerLeft(int angle, int speed) {
    Serial.print("Steering left with ");
    Serial.print(angle);
    Serial.println("° angle");
    servo.write(90 + angle);
}

void stopDriving() {
    Serial.print("Stop driving");
    digitalWrite(MOTOR_PIN_RIGHT_DIR, 0);
    analogWrite(MOTOR_PIN_RIGHT_SPEED, 0);
}

void resetSteering() {
    servo.write(90);
}
