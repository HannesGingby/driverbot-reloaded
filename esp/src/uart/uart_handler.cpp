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
          drive(255, true);
      } else if (buffedString == "x") {
          stopMotors();
      } else if (buffedString == "z") {
          // Implement stop steering if needed
      }
      // Check for the steer command (format: t,<speed>,<direction>)
      else if (buffedString.startsWith("t,")) {
          int firstComma = buffedString.indexOf(',');
          int secondComma = buffedString.indexOf(',', firstComma + 1);
          if (firstComma != -1 && secondComma != -1) {
              String speedStr = buffedString.substring(firstComma + 1, secondComma);
              String dirStr = buffedString.substring(secondComma + 1);
              int steerSpeed = speedStr.toInt();
              int steerDirection = dirStr.toInt();  // Expected to be -1 or 1

              if (steerDirection == -1) {
                  steer(steerSpeed, false);
              } else if (steerDirection == 1) {
                  steer(steerSpeed, true);
              }
          }
      }
  }
}



// void followLine() {
//     String buffedString = "";

//     // Read cx value from Maix
//     while (serial.available()) {
//         char buf = serial.read();

//         if (buf == '\n') {
//             break;  // Stop reading on \n
//         }

//         buffedString += buf;
//     }

//     if (buffedString.length() > 0) {
//         if (buffedString == "w") {
//             drive(255, true);
//         } else if (buffedString == "x") {
//             stopMotors();
//         } else if (buffedString == "t") {
//             int steerSpeed = buffedString.substring(1, 2).toInt();
//             int steerDirection = buffedString.charAt(3);

//             if (steerDirection == -1) {
//               steer(steerSpeed, false);
//             } else if (steerDirection == 1) {
//               steer(steerSpeed, true);
//             }
//         }
//     }
// }

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













// void followLine() {
//     String buffedString = "";

//     // Read cx value from Maix
//     while (serial.available()) {
//         char buf = serial.read();

//         if (buf == '\n') {
//             break;  // Stop reading on \n
//         }

//         buffedString += buf;
//     }

//     if (buffedString.length() > 0) {
//         if (buffedString == "w") {
//             driveForward(255);
//         } else if (buffedString == "x") {
//             stopDriving();
//         } else {
//             int servoAngle = buffedString.toInt();
//             if (servoAngle < 0) {
//                 steerLeft(abs(servoAngle));
//             } else if (servoAngle > 0) {
//                 steerRight(abs(servoAngle));
//             }
//             // servoAngle = constrain(servoAngle, 0, 180);  // Ensure valid range
//             // servo.write(servoAngle);
//         }
//     }
// }
