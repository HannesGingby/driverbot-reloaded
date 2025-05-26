#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <CertStoreBearSSL.h>

#include <SoftwareSerial.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <PCF8574_library.h>

#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
 #include <avr/power.h>   // Required for 16 MHz Adafruit Trinket
#endif

#include "./motor_control/motor_control.h"
#include "./mqtt/mqtt_handler.h"
#include "./uart/uart_handler.h"
#include "./credentials.h"
#include "./position_tracker/position_tracker.h"

/*

PIN SETUP

- D0 = LED light
- D1-D4 = motors
- D5/D6 = SDA/SCL for expander + laser range finders' SDA/SCL
- D7/D8 = RX/TX for UART

- P5/P6 = XSHUT for laser

*/

PCF8574 pcf8574(0x27);

VL53L0X sensor1;
VL53L0X sensor2;
bool sensor1Initialized = false;
bool sensor2Initialized = false;
unsigned long lastSensorRead = 0;
const long sensorInterval = 20;   // ms

#define I2C_SCL 14    // D5, GPIO14
#define I2C_SDA 12    // D6, GPIO12

PositionTracker positionTracker;

#define LIGHT_PIN 16   // D0, GPIO16
#define NUMPIXELS 16   // NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

const char *ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 0;
const int daylightOffsetSec = 0;

// MQTT Settings
int mqttPort = 8883;
String espClientId = "esp8266-client-" + String(WiFi.macAddress());

// Global MQTT and SSL Client
BearSSL::WiFiClientSecure espClient;
PubSubClient client(espClient);

// Global drive/steer variables
int driveDirection = 0;   // 0 = stop, 1 = forward, -1 = backward
int steerDirection = 0;   // 0 = stop, 1 = right, -1 = left
int driveSpeedPercentage = 0;
int steerSpeedPercentage = 0;


// Debug function for scanning I2C bus
void scanI2C() {
    Serial.println("Scanning I2C bus...");
    byte error, address;
    int deviceCount = 0;

    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
        Serial.print("I2C device found at address 0x");
        if (address < 16) {
            Serial.print("0");
        }
        Serial.print(address, HEX);
        Serial.println("!");
        deviceCount++;
        }
        else if (error == 4) {
        Serial.print("Unknown error at address 0x");
        if (address < 16) {
            Serial.print("0");
        }
        Serial.println(address, HEX);
        }
        yield();   // Give the watchdog timer a chance to reset
    }

    if (deviceCount == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.print("Found ");
        Serial.print(deviceCount);
        Serial.println(" device(s)");
    }
}


void setupWifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        timeout++;
        if (timeout > 20) {  // 10 second timeout
            Serial.println("\nWiFi connection timeout - restarting");
            ESP.restart();
        }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("Signal strength (RSSI): ");
    Serial.println(WiFi.RSSI());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


// Synchronize time with NTP server, required for the certificate validation
void syncTime() {
    configTime(TZ_Europe_Stockholm, ntpServer);
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    int timeout = 0;
    while (now < 8 * 3600 * 2) {
        delay(1000);
        Serial.print(".");
        now = time(nullptr);
        timeout++;
        if (timeout > 20) {
            Serial.println("\nNTP sync timeout reached.");
            break;
        }
    }
    if (now >= 8 * 3600 * 2) {
        Serial.println("\nNTP time synchronized.");
    } else {
        Serial.println("\nFailed to synchronize NTP time.");
    }
    Serial.println();

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}


void checkNTPPort() {
    WiFiClient testClient;
    Serial.println("Testing NTP port connectivity...");
    if (testClient.connect("pool.ntp.org", 123)) {
        Serial.println("NTP port seems open");
        testClient.stop();
    } else {
        Serial.println("NTP port might be blocked!");
    }
}


void setupPCF8574() {
    Serial.println("Initializing PCF8574...");

    // Quick check for PCF8574
    Wire.beginTransmission(0x27);
    byte error = Wire.endTransmission();

    if (error == 0) {
        Serial.println("PCF8574 found at address 0x27");
    } else {
        Serial.print("PCF8574 communication error: ");
        Serial.println(error);
        delay(1000);
        ESP.restart();  // Restart ESP if can't communicate with PCF8574
    }

    // Initialize with a single call
    if (pcf8574.begin()) {
        Serial.println("PCF8574 initialized successfully");

        // Configure pins as outputs with proper initial state
        // For VL53L0X, XSHUT pins should be pulled LOW to disable them initially
        pcf8574.pinMode(P5, OUTPUT, LOW);
        pcf8574.pinMode(P6, OUTPUT, LOW);

        pcf8574.digitalWrite(P5, LOW);  // Disable sensor 1
        pcf8574.digitalWrite(P6, LOW);  // Disable sensor 2
        delay(10);  // Give time for signals to stabilize
    } else {
        Serial.println("PCF8574 initialization failed");
        delay(1000);
        ESP.restart();
    }
}

// Improved setupDistanceSensors() function:
void setupDistanceSensors() {
    Serial.println("Setting up distance sensors...");

    // Make sure both sensors are off initially
    pcf8574.digitalWrite(P5, LOW);  // Disable sensor 1
    pcf8574.digitalWrite(P6, LOW);  // Disable sensor 2
    delay(50);  // Give them time to fully reset

    // Initialize sensor 1 with address 0x30
    pcf8574.digitalWrite(P5, HIGH);  // Enable sensor 1
    delay(50);  // Wait for sensor to boot up

    if (!sensor1.init()) {
        Serial.println("Sensor 1 init failed");
        sensor1Initialized = false;
    } else {
        Serial.println("Sensor 1 detected");
        sensor1.setAddress(0x30);
        sensor1.startContinuous();
        sensor1Initialized = true;
    }

    // Initialize sensor 2 with address 0x31
    pcf8574.digitalWrite(P6, HIGH);  // Enable sensor 2
    delay(50);  // Wait for sensor to boot up

    if (!sensor2.init()) {
        Serial.println("Sensor 2 init failed");
        sensor2Initialized = false;
    } else {
        Serial.println("Sensor 2 detected");
        sensor2.setAddress(0x31);
        sensor2.startContinuous();
        sensor2Initialized = true;
    }

    // Final I2C scan to confirm addresses
    Serial.println("I2C scan after sensor initialization:");
    scanI2C();
}

// In setup() function, clean up the initialization sequence:
void setup() {
    delay(500);
    serial.begin(115200);   // UART serial
    Serial.begin(115200);   // Debug serial
    delay(500);

    // Initialize WiFi and time sync
    setupWifi();
    syncTime();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MOTOR_1_DIR_PIN, OUTPUT);
    pinMode(MOTOR_1_SPEED_PIN, OUTPUT);
    pinMode(MOTOR_2_DIR_PIN, OUTPUT);
    pinMode(MOTOR_2_SPEED_PIN, OUTPUT);

    // SSL setup and MQTT client
    espClient.setTrustAnchors(new BearSSL::X509List(ca_cert));
    client.setKeepAlive(60);
    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttCallback);

    // Init I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);  // 100kHz
    Wire.setClockStretchLimit(200000);

    Serial.println("Initial I2C scan:");
    scanI2C();

    // Initialize the PCF8574 expander
    setupPCF8574();

    // Initialize the distance sensors
    setupDistanceSensors();

    // Light
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
    #endif

    pixels.begin();
    for(int i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 200));
        pixels.show();
    }

    positionTracker.begin();
}








void loop() {
    yield();

    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
    processUART();

    // Range finders
    unsigned long currentMillis = millis();
    if (currentMillis - lastSensorRead >= sensorInterval) {
        lastSensorRead = currentMillis;

        // Only try to read from sensor if both were initialized successfully
        if (sensor1Initialized && sensor2Initialized) {
            int distance1 = sensor1.readRangeContinuousMillimeters();
            int distance2 = sensor2.readRangeContinuousMillimeters();

            // Update position tracker
            positionTracker.processSensorReadings(distance1, distance2);

            float x = positionTracker.getX();               // X in mm
            float y = positionTracker.getY();               // Y in mm
            float headingRad = positionTracker.getHeading();         // Heading in radians
            float headingDeg = positionTracker.getHeadingDegrees();  // Heading in degrees

            Serial.printf("Position: (%.1f mm, %.1f mm) @ %.1f°\n", x, y, headingDeg);

            if (sensor1.timeoutOccurred()) {
                Serial.println("Sensor 1 timeout occurred");
            } else {
                Serial.print("Distance 1: ");
                Serial.print(distance1);
                Serial.println(" mm");
            }

            if (sensor2.timeoutOccurred()) {
                Serial.println("Sensor 2 timeout occurred");
            } else {
                Serial.print("Distance 2: ");
                Serial.print(distance2);
                Serial.println(" mm");
            }
        } else {
            // Attempt to reinitialize the sensor periodically
            static unsigned long lastInitAttempt = 0;
            if (currentMillis - lastInitAttempt >= 10000) {   // Try every 10 seconds
                lastInitAttempt = currentMillis;
                Serial.println("Attempting to reinitialize sensors...");
                // setupDistanceSensors();
            }
        }
    }

    // delay(10);
}






















// void setupPCF8574() {
//   Serial.println("Initializing PCF8574...");

//   // Make sure I2C is properly initialized
//   Wire.begin(I2C_SDA, I2C_SCL);

//   // Set I2C clock to a lower speed for better reliability
//   Wire.setClock(100000);  // 100kHz

//   // Quick check to see if we can communicate with PCF8574
//   Wire.beginTransmission(0x27);
//   byte error = Wire.endTransmission();

//   if (error == 0) {
//     Serial.println("PCF8574 found at address 0x27");
//   } else {
//     Serial.print("PCF8574 communication error: ");
//     Serial.println(error);
//     delay(1000);
//     ESP.restart();  // Restart ESP if can't communicate with PCF8574
//   }

//   // Configure PCF8574 pins
//   pcf8574.pinMode(P5, OUTPUT_OPEN_DRAIN);
//   pcf8574.pinMode(P6, OUTPUT_OPEN_DRAIN);

//   // Initialize the PCF8574
//   Serial.print("Init pcf8574...");
//   if (pcf8574.begin()) {
//     Serial.println("OK");
//   } else {
//     Serial.println("ERROR");
//     // Try a different approach - direct I2C writes
//     Serial.println("Trying direct I2C control...");

//     // Set all outputs high (default state)
//     Wire.beginTransmission(0x27);
//     Wire.write(0xFF);  // All pins high
//     error = Wire.endTransmission();

//     if (error == 0) {
//       Serial.println("Direct I2C control working");
//     } else {
//       Serial.println("Direct I2C control failed");
//     }
//   }
// }


// void setupDistanceSensors() {
//   if (!pcf8574.begin()) {
//     Serial.println("PCF8574 not initialized");
//     return;
//   }

//   pcf8574.digitalWrite(P5, LOW);
//   pcf8574.digitalWrite(P6, LOW);
//   delay(10);

//   // Initialize I2C
//   Wire.begin(I2C_SDA, I2C_SCL);

//   // Scan for I2C devices to debug
//   Serial.println("I2C scan after pcf8574 init:");
//   scanI2C();

//   // Init at 0x30
//   pcf8574.digitalWrite(P5, HIGH);
//   delay(10);
//   if (!sensor1.init()) {
//     Serial.println("Sensor 1 init failed");
//     return;
//   }
//   sensor1.setAddress(0x30);
//   sensor1.startContinuous();
//   sensor1Initialized = true;

//   // Init at default 0x29 ——
//   pcf8574.digitalWrite(P6, HIGH);
//   delay(10);
//   if (!sensor2.init()) {
//     Serial.println("Sensor 2 init failed");
//     return;
//   }
//   sensor2.setAddress(0x31);
//   sensor2.startContinuous();
//   sensor2Initialized = true;

//   Serial.println("Sensor 1 and 2 initialized");
// }


// void setup() {
//     delay(500);
//     serial.begin(115200);   // UART serial
//     Serial.begin(115200);   // Debug serial
//     delay(500);

//     // Initialize WiFi and time sync
//     setupWifi();
//     syncTime();

//     pinMode(LED_BUILTIN, OUTPUT);

//     pinMode(MOTOR_1_DIR_PIN, OUTPUT);
//     pinMode(MOTOR_1_SPEED_PIN, OUTPUT);
//     pinMode(MOTOR_2_DIR_PIN, OUTPUT);
//     pinMode(MOTOR_2_SPEED_PIN, OUTPUT);

//     // SSL setup and MQTT client
//     espClient.setTrustAnchors(new BearSSL::X509List(ca_cert));
//     client.setKeepAlive(60);
//     client.setServer(mqttServer, mqttPort);
//     client.setCallback(mqttCallback);

//     // // Range finders XSHUT
//     // pcf8574.pinMode(P5, OUTPUT_OPEN_DRAIN);
//     // pcf8574.pinMode(P6, OUTPUT_OPEN_DRAIN);

//     // Serial.print("Init pcf8574...");
//     // if (pcf8574.begin()){
//     //   Serial.println("OK");
//     // } else {
//     //   Serial.println("ERROR");
//     // }

//     Serial.println("Initial I2C scan:");
//     scanI2C();

//     setupPCF8574();

//     if (pcf8574.begin()) {
//       pcf8574.pinMode(P5, OUTPUT_OPEN_DRAIN);
//       pcf8574.pinMode(P6, OUTPUT_OPEN_DRAIN);
//     }

//     // Light
//     #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
//     clock_prescale_set(clock_div_1);
//     #endif

//     pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
//     for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
//         // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
//         // Here we're using a moderately bright green color:
//         pixels.setPixelColor(i, pixels.Color(255, 255, 200));
//         pixels.show();   // Send the updated pixel colors to the hardware.
//     }

//     // Range finders
//     setupDistanceSensors();

//     positionTracker.begin();
// }
