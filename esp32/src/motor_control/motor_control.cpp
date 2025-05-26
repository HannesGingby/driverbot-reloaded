#include "motor_control.h"

const int maxMotorSpeed = 255;

int clampSpeed(int speed) {
    if (speed > maxMotorSpeed) {
        speed = maxMotorSpeed;
    } else if (speed < 0) {
        speed = 0;
    }

    return speed;
}

void drive(int speed, bool direction) {
    // direction = 1, forward
    // direction = 0, backward

    digitalWrite(MOTOR_AIN1_PIN, !direction);
    digitalWrite(MOTOR_AIN2_PIN, direction);
    digitalWrite(MOTOR_BIN1_PIN, !direction);
    digitalWrite(MOTOR_BIN2_PIN, direction);

    if (0 <= speed && speed <= maxMotorSpeed) {
        ledcWrite(MOTOR_A_PWM_PIN, speed);
        ledcWrite(MOTOR_B_PWM_PIN, speed);
    } else {
        if (speed > maxMotorSpeed) {
            speed = maxMotorSpeed;
        } else if (speed < 0) {
            speed = 0;
        }
        ledcWrite(MOTOR_A_PWM_PIN, speed);
        ledcWrite(MOTOR_B_PWM_PIN, speed);
    }

    // if (direction == 1) {
    //     Serial.println("Driving forward");
    // } else {
    //     Serial.println("Driving backward");
    // }
}


void steer(int speed1, int speed2, bool direction) {
    // direction = 1, right
    // direction = 0, left

    digitalWrite(MOTOR_AIN1_PIN, !direction);
    digitalWrite(MOTOR_AIN2_PIN, direction);
    digitalWrite(MOTOR_BIN1_PIN, !direction);
    digitalWrite(MOTOR_BIN2_PIN, direction);

    if (0 <= speed1 && speed1 <= maxMotorSpeed) {
        ledcWrite(MOTOR_A_PWM_PIN, speed1);
    } else {
        if (speed1 > maxMotorSpeed) {
            speed1 = maxMotorSpeed;
        } else if (speed1 < 0) {
            speed1 = 0;
        }

        ledcWrite(MOTOR_A_PWM_PIN, speed1);
    }

    if (0 <= speed2 && speed2 <= maxMotorSpeed) {
        ledcWrite(MOTOR_B_PWM_PIN, speed2);
    } else {
        if (speed2 > maxMotorSpeed) {
            speed2 = maxMotorSpeed;
        } else if (speed2 < 0) {
            speed2 = 0;
        }

        ledcWrite(MOTOR_B_PWM_PIN, speed2);
    }

    // Serial.print("Steering in direction: ");
    // Serial.println(direction);
    // Serial.print("with speed: ");
    // Serial.println(speed1);
    // Serial.print("and speed: ");
    // Serial.println(speed2);
}


void stopMotors() {
    // Serial.print("Stopping motors");

    digitalWrite(MOTOR_AIN1_PIN, 0);
    digitalWrite(MOTOR_AIN2_PIN, 0);
    digitalWrite(MOTOR_BIN1_PIN, 0);
    digitalWrite(MOTOR_BIN2_PIN, 0);
    ledcWrite(MOTOR_A_PWM_PIN, 0);
    ledcWrite(MOTOR_B_PWM_PIN, 0);
}
