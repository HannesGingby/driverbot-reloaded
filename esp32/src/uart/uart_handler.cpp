#include "uart_handler.h"
#include "../motor_control/motor_control.h"
#include "../position_tracker/position_tracker.h"

HardwareSerial UART(2);

// Increased buffer size and added timeout handling
static char uartBuf[64];
static uint8_t uartIdx = 0;
static unsigned long lastByteTime = 0;
static const unsigned long UART_TIMEOUT = 100; // ms

extern String currentRoadTile;

void setupUART() {
    UART.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    UART.setTimeout(50); // Set timeout for UART operations

    delay(100); // Wait for UART to stabilize
}

// Internal command dispatcher
static void handleCommand(const char* cmd) {
    // Skip empty commands
    if (strlen(cmd) == 0) return;

    // Serial.printf("ESP32 Processing: '%s'\n", cmd); // Debug output

    // "w": forward
    if (strcmp(cmd, "w") == 0) {
        digitalWrite(MOTOR_AIN1_PIN, LOW);
        digitalWrite(MOTOR_AIN2_PIN, HIGH);
        digitalWrite(MOTOR_BIN1_PIN, LOW);
        digitalWrite(MOTOR_BIN2_PIN, HIGH);
        ledcWrite(MOTOR_A_PWM_PIN, 255);
        ledcWrite(MOTOR_B_PWM_PIN, 255);
        // Serial.println("Driving forward");
    }
    // "s": reverse
    else if (strcmp(cmd, "s") == 0) {
        digitalWrite(MOTOR_AIN1_PIN, HIGH);
        digitalWrite(MOTOR_AIN2_PIN, LOW);
        digitalWrite(MOTOR_BIN1_PIN, HIGH);
        digitalWrite(MOTOR_BIN2_PIN, LOW);
        ledcWrite(MOTOR_A_PWM_PIN, 255);
        ledcWrite(MOTOR_B_PWM_PIN, 255);
        // Serial.println("Driving backwards");
    }
    // "x": stop
    else if (strcmp(cmd, "x") == 0) {
        digitalWrite(MOTOR_AIN1_PIN, LOW);
        digitalWrite(MOTOR_AIN2_PIN, LOW);
        digitalWrite(MOTOR_BIN1_PIN, LOW);
        digitalWrite(MOTOR_BIN2_PIN, LOW);
        ledcWrite(MOTOR_A_PWM_PIN, 0);
        ledcWrite(MOTOR_B_PWM_PIN, 0);
        // Serial.println("Stopping motors");
    }
    // steer: t,<s1>,<d1>,<s2>,<d2>
    else if (cmd[0] == 't' && cmd[1] == ',') {
        // Serial.printf("ESP32 Steering command: %s\n", cmd);

        int s1, d1, s2, d2;

        // Use sscanf with integers directly
        if (sscanf(cmd + 2, "%d,%d,%d,%d", &s1, &d1, &s2, &d2) == 4) {
            // Clamp speeds to valid range
            s1 = clampSpeed(s1);
            s2 = clampSpeed(s2);

            // Ensure directions are 0 or 1
            d1 = (d1 != 0) ? 1 : 0;
            d2 = (d2 != 0) ? 1 : 0;

            // Set motor directions
            digitalWrite(MOTOR_AIN1_PIN, !d1);
            digitalWrite(MOTOR_AIN2_PIN, d1);
            digitalWrite(MOTOR_BIN1_PIN, !d2);
            digitalWrite(MOTOR_BIN2_PIN, d2);

            // Set motor speeds
            ledcWrite(MOTOR_A_PWM_PIN, s1);
            ledcWrite(MOTOR_B_PWM_PIN, s2);

            if (d1 != d2) {
                // Serial.println("REGISTER TICKS: FALSE");
                positionTracker.setShouldRegisterTicks(false);
            } else {
                // Serial.println("REGISTER TICKS: TRUE");
                positionTracker.setShouldRegisterTicks(true);
            }

            // Serial.printf("Motors: A(%d,%d) B(%d,%d)\n", s1, d1, s2, d2);
        } else {
            Serial.printf("Invalid steer command: %s\n", cmd);
        }
    }
    // road tile: r,<tile>
    else if (cmd[0] == 'r' && cmd[1] == ',') {
        const char* tileStr = cmd + 2;
        Serial.printf("Received tile: %s\n", tileStr);
        // TODO: sync tile with MQTT

        currentRoadTile = tileStr;
    }
    else {
        Serial.printf("Unknown command: %s\n", cmd);
    }
}

void processUART() {
    unsigned long currentTime = millis();

    // Check for timeout - if too much time passed since last byte, reset buffer
    if (uartIdx > 0 && (currentTime - lastByteTime) > UART_TIMEOUT) {
        Serial.println("UART timeout, resetting buffer");
        uartIdx = 0;
    }

    // Read all available bytes into fixed buffer
    while (UART.available()) {
        char c = UART.read();
        lastByteTime = currentTime;

        if (c == '\n') {
            if (uartIdx > 0) {
                uartBuf[uartIdx] = '\0';
                handleCommand(uartBuf);
                uartIdx = 0;
            }
        } else if (c >= 32 && c <= 126) { // Only accept printable ASCII characters
            if (uartIdx < (sizeof(uartBuf) - 1)) {
                uartBuf[uartIdx++] = c;
            } else {
                // Buffer overflow, reset
                Serial.println("UART buffer overflow, resetting");
                uartIdx = 0;
            }
        }
        // Ignore non-printable characters
    }
}

void sendPosition(int tileX, int tileY) {
    Serial.printf("ESP32 Sending position: %d, %d\n", tileX, tileY);

    char msg[32];
    int len = snprintf(msg, sizeof(msg), "p,%d,%d\n", tileX, tileY);
    if (len > 0 && len < sizeof(msg)) {
        UART.write((const uint8_t*)msg, len);
        UART.flush(); // Ensure data is sent immediately
        delay(10); // Small delay to prevent overwhelming the receiver
    }
}

void sendCommand(String command, int startPosX, int startPosY, int mapSizeX, int mapSizeY) {
    char msg[32];
    int len = snprintf(msg, sizeof(msg), "c,%s,%d,%d,%d,%d\n", command.c_str(), startPosX, startPosY, mapSizeX, mapSizeY);
    if (len > 0 && len < sizeof(msg)) {
        UART.write((const uint8_t*)msg, len);
        UART.flush(); // Ensure data is sent immediately
        delay(10); // Small delay to prevent overwhelming the receiver
    }

    Serial.print("Sending command");
    Serial.println(command);
}

void maixLed() {
    UART.write("r\n");
    UART.flush();
    delay(1000);
    UART.write("g\n");
    UART.flush();
    delay(1000);
    UART.write("b\n");
    UART.flush();
    delay(1000);
}











// #include "uart_handler.h"
// #include "../motor_control/motor_control.h"

// HardwareSerial UART(2);

// // Fixed-size buffer for incoming UART data
// static char uartBuf[32];
// static uint8_t uartIdx = 0;

// void setupUART() {
//     UART.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
//     // Signal ready to Maix
//     UART.write("f\n");
// }

// // Internal command dispatcher
// static void handleCommand(const char* cmd) {
//     // "w": forward
//     if (strcmp(cmd, "w") == 0) {
//         digitalWrite(MOTOR_AIN1_PIN, LOW);
//         digitalWrite(MOTOR_AIN2_PIN, HIGH);
//         digitalWrite(MOTOR_BIN1_PIN, LOW);
//         digitalWrite(MOTOR_BIN2_PIN, HIGH);
//         ledcWrite(MOTOR_A_PWM_PIN, 255);
//         ledcWrite(MOTOR_B_PWM_PIN, 255);
//         // Serial.println("Driving forward");
//     }
//     // "s": reverse
//     else if (strcmp(cmd, "s") == 0) {
//         digitalWrite(MOTOR_AIN1_PIN, HIGH);
//         digitalWrite(MOTOR_AIN2_PIN, LOW);
//         digitalWrite(MOTOR_BIN1_PIN, HIGH);
//         digitalWrite(MOTOR_BIN2_PIN, LOW);
//         ledcWrite(MOTOR_A_PWM_PIN, 255);
//         ledcWrite(MOTOR_B_PWM_PIN, 255);
//         // Serial.println("Driving backwards");
//     }
//     // "x": stop
//     else if (strcmp(cmd, "x") == 0) {
//         digitalWrite(MOTOR_AIN1_PIN, LOW);
//         digitalWrite(MOTOR_AIN2_PIN, LOW);
//         digitalWrite(MOTOR_BIN1_PIN, LOW);
//         digitalWrite(MOTOR_BIN2_PIN, LOW);
//         ledcWrite(MOTOR_A_PWM_PIN, 0);
//         ledcWrite(MOTOR_B_PWM_PIN, 0);
//         // Serial.println("Stopping motors");
//     }
//     // steer: t,<s1>,<d1>,<s2>,<d2>
//     else if (cmd[0] == 't' && cmd[1] == ',') {
//         // Serial.println("Steering");
//         // Serial.println(cmd);
//         float s1_f, s2_f;
//         int d1, d2;

//         if (sscanf(cmd + 2, "%f,%d,%f,%d", &s1_f, &d1, &s2_f, &d2) == 4) {
//             // Convert floats to integers
//             int s1 = (int)s1_f;
//             int s2 = (int)s2_f;

//             // Clamp speeds to valid range
//             s1 = clampSpeed(s1);
//             s2 = clampSpeed(s2);

//             // Set motor directions
//             digitalWrite(MOTOR_AIN1_PIN, !d1);
//             digitalWrite(MOTOR_AIN2_PIN, d1);
//             digitalWrite(MOTOR_BIN1_PIN, !d2);
//             digitalWrite(MOTOR_BIN2_PIN, d2);

//             // Set motor speeds
//             ledcWrite(MOTOR_A_PWM_PIN, s1);
//             ledcWrite(MOTOR_B_PWM_PIN, s2);

//             // Serial.printf("Driving with directions: %d, %d\n", d1, d2);
//             // Serial.printf("Driving with speeds: %d, %d\n", s1, s2);
//         } else {
//             Serial.println("Invalid turn command format");
//         }
//     }
//     // road tile: r,<tile>
//     else if (cmd[0] == 'r' && cmd[1] == ',') {
//         const char* tileStr = cmd + 2;
//         Serial.printf("Received tile: %s\n", tileStr);
//         // TODO: sync tile with MQTT
//     }
// }

// void processUART() {
//     // Read all available bytes into fixed buffer
//     while (UART.available()) {
//         char c = UART.read();
//         if (c == '\n') {
//             uartBuf[uartIdx] = '\0';
//             if (uartIdx > 0) {
//                 handleCommand(uartBuf);
//             }
//             uartIdx = 0;
//         } else if (uartIdx < (sizeof(uartBuf) - 1)) {
//             uartBuf[uartIdx++] = c;
//         }
//         // if overflow, silently drop excess
//     }
// }

// void sendPosition(int tileX, int tileY) {
//     Serial.println("Sending position");

//     char msg[32];
//     int len = snprintf(msg, sizeof(msg), "p,%d,%d\n", tileX, tileY);
//     if (len > 0) {
//         UART.write((uint8_t*)msg, len);
//     }
// }

// void maixLed() {
//     UART.write("r\n");
//     delay(1000);
//     UART.write("g\n");
//     delay(1000);
//     UART.write("b\n");
//     delay(1000);
// }












// #include "uart_handler.h"
// #include "../motor_control/motor_control.h"

// HardwareSerial UART(2);

// void setupUART() {
//     UART.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

//     UART.write("f\n");
// }


// void processUART() {
//     String buffedString = "";

//     // Read a complete command until newline is encountered
//     while (UART.available()) {
//         char buf = UART.read();

//         if (buf == '\n') {
//             break;  // End of command
//         }
//         buffedString += buf;
//     }

//     if (buffedString.length() > 0) {
//         // Check for the forward and stop commands
//         if (buffedString == "w") {
//             // drive(255, true);
//             digitalWrite(MOTOR_AIN1_PIN, 0);
//             digitalWrite(MOTOR_AIN2_PIN, 1);
//             digitalWrite(MOTOR_BIN1_PIN, 0);
//             digitalWrite(MOTOR_BIN2_PIN, 1);
//             ledcWrite(MOTOR_A_PWM_PIN, 255);
//             ledcWrite(MOTOR_B_PWM_PIN, 255);
//             Serial.println("Driving forward");
//         } else if (buffedString == "s") {
//             // drive(255, false);
//             digitalWrite(MOTOR_AIN1_PIN, 1);
//             digitalWrite(MOTOR_AIN2_PIN, 0);
//             digitalWrite(MOTOR_BIN1_PIN, 1);
//             digitalWrite(MOTOR_BIN2_PIN, 0);
//             ledcWrite(MOTOR_A_PWM_PIN, 255);
//             ledcWrite(MOTOR_B_PWM_PIN, 255);
//             Serial.println("Driving backwards");
//         } else if (buffedString == "x") {
//             // stopMotors();
//             digitalWrite(MOTOR_AIN1_PIN, 0);
//             digitalWrite(MOTOR_AIN2_PIN, 0);
//             digitalWrite(MOTOR_BIN1_PIN, 0);
//             digitalWrite(MOTOR_BIN2_PIN, 0);
//             ledcWrite(MOTOR_A_PWM_PIN, 0);
//             ledcWrite(MOTOR_B_PWM_PIN, 0);
//             Serial.println("Stopping motors");
//         } else if (buffedString.startsWith("t,")) {   // Check for the steer command (format: t,<speed1>,<direction1>,<speed2>,<direction2>)
//             Serial.println("Steering");

//             // find indices of the four commas
//             int c1 = buffedString.indexOf(',');
//             int c2 = buffedString.indexOf(',', c1 + 1);
//             int c3 = buffedString.indexOf(',', c2 + 1);
//             int c4 = buffedString.indexOf(',', c3 + 1);

//             // make sure we got all four commas
//             if (c1 != -1 && c2 != -1 && c3 != -1 && c4 != -1) {
//                 // extract each field
//                 String speedStr1 = buffedString.substring(c1 + 1, c2);
//                 String dirStr1   = buffedString.substring(c2 + 1, c3);
//                 String speedStr2 = buffedString.substring(c3 + 1, c4);
//                 String dirStr2   = buffedString.substring(c4 + 1);

//                 // convert to integers
//                 int speed1     = speedStr1.toInt();
//                 int direction1 = dirStr1.toInt();   // expected 0 or 1
//                 int speed2     = speedStr2.toInt();
//                 int direction2 = dirStr2.toInt();   // expected 0 or 1

//                 speed1 = clampSpeed(speed1);
//                 speed2 = clampSpeed(speed2);

//                 // Serial.printf("Driving with directions: %d, %d\n", direction1, direction2);
//                 // Serial.printf("Driving with speeds: %d, %d\n", speed1, speed2);

//                 digitalWrite(MOTOR_AIN1_PIN, !direction1);
//                 digitalWrite(MOTOR_AIN2_PIN, direction1);
//                 digitalWrite(MOTOR_BIN1_PIN, !direction2);
//                 digitalWrite(MOTOR_BIN2_PIN, direction2);
//                 ledcWrite(MOTOR_A_PWM_PIN, speed1);
//                 ledcWrite(MOTOR_B_PWM_PIN, speed2);
//             }
//         } else if (buffedString.startsWith("r,")) {   // Check for road tile command (format: r,<roadTile>)
//             String tileStr = buffedString.substring(2);
//             Serial.printf("Received tile: %s\n", tileStr.c_str());

//             // Sync tile with MQTT

//         }
//     }
// }


// void sendPosition(int tileX, int tileY) {
//     String message = "p," + String(tileX) + "," + String(tileY) + "\n";
//     UART.write(message.c_str());
// }


// void maixLed() {
//     UART.write("r\n");
//     delay(1000);
//     UART.write("g\n");
//     delay(1000);
//     UART.write("b\n");
//     delay(1000);
// }
