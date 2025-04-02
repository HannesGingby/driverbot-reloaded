#include "motor_control.h"

const int maxMotorSpeed = 255;
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
    Serial.print("Stopping motors");

    analogWrite(MOTOR_1_SPEED_PIN, 0);
    analogWrite(MOTOR_2_SPEED_PIN, 0);
    digitalWrite(MOTOR_1_DIR_PIN, 0);
    digitalWrite(MOTOR_2_DIR_PIN, 0);

    // // If not steering, stop the motors
    // if (!isSteering) {
    //     analogWrite(MOTOR_1_SPEED_PIN, 0);
    //     analogWrite(MOTOR_2_SPEED_PIN, 0);
    // }
}

// void stopSteering() {
//     isSteering = false;
//     stopMotors();
// }















// #include "motor_control.h"

// Servo servo;
// const int maxMotorSpeed = 255;
// bool isSteering = false;

// void driveForward(int speed) {
//     if (isSteering) {
//         if (0 <= speed && speed <= maxMotorSpeed) {
//             Serial.print("Driving forward with the speed of ");
//             Serial.println(speed);

//             analogWrite(MOTOR_1_SPEED_PIN, speed);
//             digitalWrite(MOTOR_1_DIR_PIN, 1);
//         } else {
//             Serial.println("Not driving forward, 'speed' is out of range");
//         }
//     }
// }

// void driveBackward(int speed) {
//     if (isSteering) {
//         if (0 <= speed && speed <= maxMotorSpeed) {
//             Serial.print("Driving backward with the speed of ");
//             Serial.println(speed);

//             analogWrite(MOTOR_1_SPEED_PIN, speed);
//             digitalWrite(MOTOR_1_DIR_PIN, 0);
//         } else {
//             Serial.println("Not driving backward, 'speed' is out of range");
//         }
//     }
// }

// void steerRight(int angle) {
//     Serial.print("Steering right with ");
//     Serial.print(angle);
//     Serial.println("° angle");

//     isSteering = true;
//     int servoOutput = 90 - angle;

//     if (servoOutput < 0) {
//         Serial.println("'servoOutput' is negative, clamping to 0");
//         servoOutput = 0;
//     } else if (servoOutput > 180) {
//         Serial.println("'servoOutput' is greater than 180, clamping to 180");
//         servoOutput = 180;
//     }

//     //servo.write(servoOutput);

//     // todo: add steering code
// }

// void steerLeft(int angle) {
//     Serial.print("Steering left with ");
//     Serial.print(angle);
//     Serial.println("° angle");

//     isSteering = true;
//     int servoOutput = 90 + angle;

//     if (servoOutput < 0) {
//         Serial.println("'servoOutput' is negative, clamping to 0");
//         servoOutput = 0;
//     } else if (servoOutput > 180) {
//         Serial.println("'servoOutput' is greater than 180, clamping to 180");
//         servoOutput = 180;
//     }

//     //servo.write(servoOutput);

//     // todo: add steering code
// }

// void stopDriving() {
//     Serial.print("Stop driving");
//     analogWrite(MOTOR_1_SPEED_PIN, 0);
// }

// void resetSteering() {
//     isSteering = false;

//     // todo: add steering code
// }
























// void driveForward(int speed) {
//     Serial.print("Driving forward with the speed of ");
//     Serial.println(speed);

//     analogWrite(MOTOR_PIN_RIGHT_SPEED, 1);
//     digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);

//     // digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);
//     // analogWrite(MOTOR_PIN_RIGHT_SPEED, speed);
// }

// void driveBackward(int speed) {
//     Serial.print("Driving backward with the speed of ");
//     Serial.println(speed);

//     analogWrite(MOTOR_PIN_RIGHT_SPEED, 1);
//     digitalWrite(MOTOR_PIN_RIGHT_DIR, 0);

//     //digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);
//     //analogWrite(MOTOR_PIN_RIGHT_SPEED, speed);
// }

// void steerRight(int angle) {
//     Serial.print("Steering right with ");
//     Serial.print(angle);
//     Serial.println("° angle");
//     // Directly set the servo angle
//     servo.write(90 - angle);
// }

// void steerLeft(int angle) {
//     Serial.print("Steering left with ");
//     Serial.print(angle);
//     Serial.println("° angle");
//     servo.write(90 + angle);
// }

// void stopDriving() {
//     Serial.print("Stop driving");
//     digitalWrite(MOTOR_PIN_RIGHT_DIR, 0);
//     analogWrite(MOTOR_PIN_RIGHT_SPEED, 0);
// }

// void resetSteering() {
//     servo.write(90);
// }
