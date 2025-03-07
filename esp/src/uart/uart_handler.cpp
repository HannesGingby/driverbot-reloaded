#include "uart_handler.h"
#include "../motor_control/motor_control.h"

SoftwareSerial serial(4, 5); // RX, TX

void setupUART() {
    serial.begin(115200);

    serial.write("f\n");
}

void followLine() {
    String buffedString = "";

    // Read cx value from Maix
    while (serial.available()) {
        char buf = serial.read();

        if (buf == '\n') {
            break;  // Stop reading on \n
        }

        buffedString += buf;
    }

    if (buffedString.length() > 0) {
        if (buffedString == "w") {
            driveForward(100);
        } else {
            int servoAngle = buffedString.toInt();
            servoAngle = constrain(servoAngle, 0, 180);  // Ensure valid range
            servo.write(servoAngle);
        }
    }
}

void processUART() {
    followLine();
}

void maixLed() {
    serial.write("r\n");
    delay(1000);
    serial.write("g\n");
    delay(1000);
    serial.write("b\n");
    delay(1000);
}
