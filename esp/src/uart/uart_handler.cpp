#include "uart_handler.h"
#include "../motor_control/motor_control.h"

SoftwareSerial serial(13, 15); // RX, TX   (D7, D8)

void setupUART() {
    serial.begin(115200);

    serial.write("f\n");
}

void followLine() {
  String buffedString = "";

  // Read a complete command until newline is encountered
  while (serial.available()) {
      char buf = serial.read();

      if (buf == '\n') {
          break;  // End of command
      }
      buffedString += buf;
  }

  if (buffedString.length() > 0) {
      // Check for the forward and stop commands
      if (buffedString == "w") {
          Serial.println("Driving forward");
          drive(400, true);
      } else if (buffedString == "s") {
          Serial.println("Driving backward");
          drive(400, false);
      } else if (buffedString == "x") {
          stopMotors();
      }
      // Check for the steer command (format: t,<speed1>,<direction1>,<speed2>,<direction2>)
      else if (buffedString.startsWith("t,")) {
          // find indices of the four commas
          int c1 = buffedString.indexOf(',');
          int c2 = buffedString.indexOf(',', c1 + 1);
          int c3 = buffedString.indexOf(',', c2 + 1);
          int c4 = buffedString.indexOf(',', c3 + 1);

          // make sure we got all four commas
          if (c1 != -1 && c2 != -1 && c3 != -1 && c4 != -1) {
              // extract each field
              String speedStr1 = buffedString.substring(c1 + 1, c2);
              String dirStr1   = buffedString.substring(c2 + 1, c3);
              String speedStr2 = buffedString.substring(c3 + 1, c4);
              String dirStr2   = buffedString.substring(c4 + 1);

              // convert to integers
              int speed1     = speedStr1.toInt();
              int direction1 = dirStr1.toInt();   // expected -1 or 1
              int speed2     = speedStr2.toInt();
              int direction2 = dirStr2.toInt();   // expected -1 or 1

              if (direction1 == 1 && direction2 == 1) {
                analogWrite(MOTOR_1_SPEED_PIN, speed1);
                digitalWrite(MOTOR_1_DIR_PIN, 1);

                analogWrite(MOTOR_2_SPEED_PIN, speed2);
                digitalWrite(MOTOR_2_DIR_PIN, 1);
              } else if (direction1 == 1 && direction2 == -1) {
                analogWrite(MOTOR_1_SPEED_PIN, speed1);
                digitalWrite(MOTOR_1_DIR_PIN, 1);

                analogWrite(MOTOR_2_SPEED_PIN, speed2);
                digitalWrite(MOTOR_2_DIR_PIN, 0);
              } else if (direction1 == -1 && direction2 == 1) {
                analogWrite(MOTOR_1_SPEED_PIN, speed1);
                digitalWrite(MOTOR_1_DIR_PIN, 0);

                analogWrite(MOTOR_2_SPEED_PIN, speed2);
                digitalWrite(MOTOR_2_DIR_PIN, 1);
              } else if (direction1 == -1 && direction2 == -1) {
                analogWrite(MOTOR_1_SPEED_PIN, speed1);
                digitalWrite(MOTOR_1_DIR_PIN, 0);

                analogWrite(MOTOR_2_SPEED_PIN, speed2);
                digitalWrite(MOTOR_2_DIR_PIN, 0);
              }
          }
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
